#include "foundation/logging/log_file_sink.h"

#include "foundation/logging/log_redactor.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QThread>

#include <utility>

namespace player::logging {

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
        if (!openFileLocked(errorMessage)) {
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
    return QDir(m_options.directory).filePath(m_options.fileName);
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
        // file when it can be reopened instead of recursively logging errors.
    }

    if (m_file.write(encoded) != encoded.size()) {
        setLastErrorLocked(m_file.errorString());
    }
}

bool LogFileSink::openFileLocked(QString* errorMessage)
{
    m_file.setFileName(currentLogFilePath());

    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
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
    const QString activePath = currentLogFilePath();

    if (m_file.isOpen()) {
        m_file.flush();
        m_file.close();
    }

    QDir directory(m_options.directory);

    if (m_options.maxArchives == 0) {
        if (!QFile::remove(activePath) && QFileInfo::exists(activePath)) {
            setLastErrorLocked(QStringLiteral("Unable to remove log file during rotation: %1").arg(activePath));
            return openFileLocked(nullptr);
        }
    }
    else {
        const QString oldestPath = directory.filePath(
            QStringLiteral("%1.%2").arg(m_options.fileName).arg(m_options.maxArchives));
        if (QFileInfo::exists(oldestPath) && !QFile::remove(oldestPath)) {
            setLastErrorLocked(QStringLiteral("Unable to remove oldest log archive: %1").arg(oldestPath));
            return openFileLocked(nullptr);
        }

        for (int index = m_options.maxArchives - 1; index >= 1; --index) {
            const QString source = directory.filePath(
                QStringLiteral("%1.%2").arg(m_options.fileName).arg(index));
            const QString destination = directory.filePath(
                QStringLiteral("%1.%2").arg(m_options.fileName).arg(index + 1));

            if (QFileInfo::exists(source) && !QFile::rename(source, destination)) {
                setLastErrorLocked(QStringLiteral("Unable to rotate log archive: %1").arg(source));
                return openFileLocked(nullptr);
            }
        }

        const QString firstArchive = directory.filePath(
            QStringLiteral("%1.1").arg(m_options.fileName));
        if (QFileInfo::exists(activePath) && !QFile::rename(activePath, firstArchive)) {
            setLastErrorLocked(QStringLiteral("Unable to rotate active log file: %1").arg(activePath));
            return openFileLocked(nullptr);
        }
    }

    return openFileLocked(nullptr);
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
