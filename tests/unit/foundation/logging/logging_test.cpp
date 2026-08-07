#include "foundation/logging/log_categories.h"
#include "foundation/logging/log_file_sink.h"
#include "foundation/logging/log_redactor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

#include <thread>
#include <vector>

namespace {

QString readTextFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

class LoggingTest final : public QObject
{
    Q_OBJECT

private slots:
    void redactsSecrets();
    void writesThroughQtHandlerAndFlushesOnStop();
    void writesFromMultipleThreads();
    void rotatesFiles();
    void reportsInvalidLogDirectory();
};

void LoggingTest::redactsSecrets()
{
    const QString input = QStringLiteral(
        "https://example.test/video?token=secret&api_key=abc Authorization=xyz Bearer bearer-secret");
    const QString redacted = player::logging::LogRedactor::redact(input);

    QVERIFY(!redacted.contains(QStringLiteral("secret")));
    QVERIFY(!redacted.contains(QStringLiteral("abc")));
    QVERIFY(!redacted.contains(QStringLiteral("xyz")));
    QVERIFY(redacted.contains(QStringLiteral("<redacted>")));
}

void LoggingTest::writesThroughQtHandlerAndFlushesOnStop()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    player::logging::LogFileSinkOptions options;
    options.directory = temporaryDirectory.path();

    player::logging::LogFileSink sink(options);
    QString error;
    QVERIFY2(sink.start(&error), qPrintable(error));

    qCInfo(player::logging::appLifecycle).noquote()
        << "handler-message token=do-not-write-this";

    sink.stop();

    const QString content = readTextFile(
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("player.log")));
    QVERIFY(content.contains(QStringLiteral("handler-message")));
    QVERIFY(!content.contains(QStringLiteral("do-not-write-this")));
    QVERIFY(content.contains(QStringLiteral("<redacted>")));
}

void LoggingTest::writesFromMultipleThreads()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    player::logging::LogFileSinkOptions options;
    options.directory = temporaryDirectory.path();
    options.maxBytes = 8 * 1024 * 1024;

    player::logging::LogFileSink sink(options);
    QString error;
    QVERIFY2(sink.start(&error), qPrintable(error));

    constexpr int threadCount = 4;
    constexpr int messagesPerThread = 50;

    std::vector<std::thread> workers;
    workers.reserve(threadCount);
    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        workers.emplace_back([&sink, threadIndex]() {
            for (int messageIndex = 0; messageIndex < messagesPerThread; ++messageIndex) {
                sink.writeMessage(
                    QtInfoMsg,
                    QMessageLogContext(),
                    QStringLiteral("worker-%1-message-%2")
                        .arg(threadIndex)
                        .arg(messageIndex));
            }
        });
    }

    for (std::thread& worker : workers) {
        worker.join();
    }

    sink.stop();

    const QString content = readTextFile(
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("player.log")));
    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        for (int messageIndex = 0; messageIndex < messagesPerThread; ++messageIndex) {
            QVERIFY(content.contains(
                QStringLiteral("worker-%1-message-%2")
                    .arg(threadIndex)
                    .arg(messageIndex)));
        }
    }
}

void LoggingTest::rotatesFiles()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    player::logging::LogFileSinkOptions options;
    options.directory = temporaryDirectory.path();
    options.maxBytes = 256;
    options.maxArchives = 2;

    player::logging::LogFileSink sink(options);
    QString error;
    QVERIFY2(sink.start(&error), qPrintable(error));

    for (int index = 0; index < 40; ++index) {
        sink.writeMessage(
            QtInfoMsg,
            QMessageLogContext(),
            QStringLiteral("rotation-message-%1-abcdefghijklmnopqrstuvwxyz").arg(index));
    }

    sink.stop();

    const QDir directory(temporaryDirectory.path());
    QVERIFY(QFileInfo::exists(directory.filePath(QStringLiteral("player.log"))));
    QVERIFY(QFileInfo::exists(directory.filePath(QStringLiteral("player.log.1"))));
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("player.log.3"))));
}

void LoggingTest::reportsInvalidLogDirectory()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString blockingFile = QDir(temporaryDirectory.path()).filePath(QStringLiteral("blocking-file"));
    QFile file(blockingFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("x");
    file.close();

    player::logging::LogFileSinkOptions options;
    options.directory = QDir(blockingFile).filePath(QStringLiteral("logs"));

    player::logging::LogFileSink sink(options);
    QString error;
    QVERIFY(!sink.start(&error));
    QVERIFY(!error.isEmpty());
}

QTEST_MAIN(LoggingTest)
#include "logging_test.moc"
