#pragma once

#include <QFile>
#include <QMessageLogContext>
#include <QMutex>
#include <QString>

namespace player::logging {

struct LogFileSinkOptions final
{
    QString directory;
    QString fileName = QStringLiteral("player.log");
    qint64 maxBytes = 4 * 1024 * 1024;
    int maxArchives = 3;
};

class LogFileSink final
{
public:
    explicit LogFileSink(LogFileSinkOptions options);
    ~LogFileSink();

    LogFileSink(const LogFileSink&) = delete;
    LogFileSink& operator=(const LogFileSink&) = delete;

    [[nodiscard]] bool start(QString* errorMessage = nullptr);
    void stop();
    [[nodiscard]] bool flush(QString* errorMessage = nullptr);

    [[nodiscard]] bool isActive() const;
    [[nodiscard]] QString currentLogFilePath() const;
    [[nodiscard]] QString lastError() const;

    void writeMessage(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& message);

private:
    [[nodiscard]] bool openNewSessionFileLocked(QString* errorMessage);
    [[nodiscard]] bool openCurrentFileLocked(
        bool appendExisting,
        QString* errorMessage);
    [[nodiscard]] bool rotateIfNeededLocked(qint64 incomingBytes);
    [[nodiscard]] bool rotateLocked();
    void setLastErrorLocked(QString message);

    [[nodiscard]] static QString severityName(QtMsgType type);
    static void qtMessageHandler(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& message);

    LogFileSinkOptions m_options;
    mutable QMutex m_mutex;
    QFile m_file;
    QString m_currentLogFilePath;
    QString m_lastError;
    QtMessageHandler m_previousHandler = nullptr;
    bool m_active = false;

    static QMutex s_handlerMutex;
    static LogFileSink* s_activeSink;
};

} // namespace player::logging
