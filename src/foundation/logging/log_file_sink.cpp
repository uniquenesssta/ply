#include "foundation/logging/log_file_sink.h"

#include "foundation/logging/log_redactor.h"
#include "foundation/logging/log_session_file_name.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QThread>

#include <utility>

namespace player::logging {
namespace {

constexpr int kMaximumSessionFileNameAttempts = 999;

} // namespace

QMutex LogFileSink::s_handlerMutex;
LogFileSink* LogFileSink::s_activeSink = nullptr;

LogFileSink::LogFileSink(LogFileSinkOptions options)
    : m_options(std::move(options))
{
}

LogFileSink::~LogFileSink()
{
    stop();
}

bool LogFileSink::start(QString* errorMessage)
{
    QMutexLocker handlerLocker(&s_handlerMutex);

    if (m_active) {
        return true;
    }

    if (s_activeSink != nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Another LogFileSink is already active.");
        }
        return false;
    }

    if (m_options.directory.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Log directory is empty.");
        }
        return false;
    }

    if (m_options.maxBytes <= 0 || m_options.maxArchives < 0) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Log rotation options are invalid.");
        }
        return false;
    }

    if (!QDir().mkpath(m_options.directory)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to create log directory: %1").arg(m_options.directory);
        }
        return false;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!openNewSessionFileLocked(errorMessage)) {
            return false;
        }
    }

    s_activeSink = this;
    m_previousHandler = qInstallMessageHandler(&LogFileSink::qtMessageHandler);
    m_active = true;
    return true;
}

void LogFileSink::stop()
{
    QMutexLocker handlerLocker(&s_handlerMutex);

    if (!m_active) {
        return;
    }

    qInstallMessageHandler(m_previousHandler);
    s_activeSink = nullptr;
    m_active = false;

    QMutexLocker locker(&m_mutex);
    if (m_file.isOpen()) {
        if (!m_file.flush()) {
            setLastErrorLocked(m_file.errorString());
        }
        m_file.close();
    }
}

bool LogFileSink::flush(QString* errorMessage)
{
    QMutexLocker locker(&m_mutex);

    if (!m_file.isOpen()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Log file is not open.");
        }
        return false;
    }

    if (!m_file.flush()) {
        setLastErrorLocked(m_file.errorString());
        if (errorMessage != nullptr) {
            *errorMessage = m_lastError;
        }
        return false;
    }

    return true;
}

bool LogFileSink::isActive() const
{
    QMutexLocker handlerLocker(&s_handlerMutex);
    return m_active;
}

QString LogFileSink::currentLogFilePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentLogFilePath;
}

QString LogFileSink::lastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

void LogFileSink::writeMessage(
    QtMsgType type,
    const QMessageLogContext& context,
    const QString& message)
{
    const QString category = context.category != nullptr
        ? QString::fromUtf8(context.category)
        : QStringLiteral("default");
    const QString fileName = context.file != nullptr
        ? QFileInfo(QString::fromUtf8(context.file)).fileName()
        : QStringLiteral("-");
    const QString functionName = context.function != nullptr
        ? QString::fromUtf8(context.function)
        : QStringLiteral("-");

    const QString safeMessage = LogRedactor::redact(message);
    const QString line = QStringLiteral("%1 [%2] [0x%3] [%4] %5 (%6:%7 %8)\n")
        .arg(
            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),
            severityName(type),
            QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16),
            category,
            safeMessage,
            fileName,
            QString::number(context.line),
            functionName);

    const QByteArray encoded = line.toUtf8();

    QMutexLocker locker(&m_mutex);
    if (!m_file.isOpen()) {
        return;
    }

    if (!rotateIfNeededLocked(encoded.size())) {
        // Rotation failure is stored in lastError. Keep logging to the active
        // session file when it can be reopened instead of recursively logging errors.
    }

    if (m_file.write(encoded) != encoded.size()) {
        setLastErrorLocked(m_file.errorString());
    }
}

bool LogFileSink::openNewSessionFileLocked(QString* errorMessage)
{
    const QDateTime sessionStartedAt = QDateTime::currentDateTime();
    const QDir directory(m_options.directory);

    for (int sequence = 1; sequence <= kMaximumSessionFileNameAttempts; ++sequence) {
        const QString candidatePath = directory.filePath(
            makeLogSessionFileName(m_options.fileName, sessionStartedAt, sequence));
        if (QFileInfo::exists(candidatePath)) {
            continue;
        }

        m_file.setFileName(candidatePath);
        if (m_file.open(QIODevice::WriteOnly | QIODevice::NewOnly | QIODevice::Text)) {
            m_currentLogFilePath = candidatePath;
            m_lastError.clear();
            return true;
        }

        if (QFileInfo::exists(candidatePath)) {
            continue;
        }

        setLastErrorLocked(m_file.errorString());
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to create log session file '%1': %2")
                                .arg(candidatePath, m_lastError);
        }
        return false;
    }

    setLastErrorLocked(QStringLiteral("Unable to allocate a unique log session file name."));
    if (errorMessage != nullptr) {
        *errorMessage = m_lastError;
    }
    return false;
}

bool LogFileSink::openCurrentFileLocked(
    bool appendExisting,
    QString* errorMessage)
{
    if (m_currentLogFilePath.isEmpty()) {
        setLastErrorLocked(QStringLiteral("Current log session file path is empty."));
        if (errorMessage != nullptr) {
            *errorMessage = m_lastError;
        }
        return false;
    }

    m_file.setFileName(m_currentLogFilePath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    mode |= appendExisting ? QIODevice::Append : QIODevice::NewOnly;

    if (!m_file.open(mode)) {
        setLastErrorLocked(m_file.errorString());
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unable to open log file '%1': %2")
                                .arg(m_file.fileName(), m_lastError);
        }
        return false;
    }

    return true;
}

bool LogFileSink::rotateIfNeededLocked(qint64 incomingBytes)
{
    if (m_file.size() + incomingBytes <= m_options.maxBytes) {
        return true;
    }

    return rotateLocked();
}

bool LogFileSink::rotateLocked()
{
    const QString activePath = m_currentLogFilePath;

    if (m_file.isOpen()) {
        m_file.flush();
        m_file.close();
    }

    if (m_options.maxArchives == 0) {
        if (!QFile::remove(activePath) && QFileInfo::exists(activePath)) {
            setLastErrorLocked(QStringLiteral("Unable to remove log file during rotation: %1").arg(activePath));
            return openCurrentFileLocked(true, nullptr);
        }
    }
    else {
        const QString oldestPath = QStringLiteral("%1.%2")
                                       .arg(activePath)
                                       .arg(m_options.maxArchives);
        if (QFileInfo::exists(oldestPath) && !QFile::remove(oldestPath)) {
            setLastErrorLocked(QStringLiteral("Unable to remove oldest log archive: %1").arg(oldestPath));
            return openCurrentFileLocked(true, nullptr);
        }

        for (int index = m_options.maxArchives - 1; index >= 1; --index) {
            const QString source = QStringLiteral("%1.%2").arg(activePath).arg(index);
            const QString destination = QStringLiteral("%1.%2").arg(activePath).arg(index + 1);

            if (QFileInfo::exists(source) && !QFile::rename(source, destination)) {
                setLastErrorLocked(QStringLiteral("Unable to rotate log archive: %1").arg(source));
                return openCurrentFileLocked(true, nullptr);
            }
        }

        const QString firstArchive = QStringLiteral("%1.1").arg(activePath);
        if (QFileInfo::exists(activePath) && !QFile::rename(activePath, firstArchive)) {
            setLastErrorLocked(QStringLiteral("Unable to rotate active log file: %1").arg(activePath));
            return openCurrentFileLocked(true, nullptr);
        }
    }

    return openCurrentFileLocked(false, nullptr);
}

void LogFileSink::setLastErrorLocked(QString message)
{
    m_lastError = std::move(message);
}

QString LogFileSink::severityName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARN");
    case QtCriticalMsg:
        return QStringLiteral("ERROR");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }

    return QStringLiteral("UNKNOWN");
}

void LogFileSink::qtMessageHandler(
    QtMsgType type,
    const QMessageLogContext& context,
    const QString& message)
{
    QMutexLocker handlerLocker(&s_handlerMutex);
    if (s_activeSink != nullptr) {
        s_activeSink->writeMessage(type, context, message);
    }
}

} // namespace player::logging
