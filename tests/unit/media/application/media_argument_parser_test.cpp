#include "media/application/arguments/media_argument_parser.h"

#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

namespace player::media::application {

class MediaArgumentParserTest final : public QObject
{
    Q_OBJECT

private slots:
    void noMediaArgumentsProducesEmptyResult();
    void localPathsResolveAgainstWorkingDirectory();
    void remoteUrlsRemainUrls();
    void multipleArgumentsPreserveOrder();
    void optionLikeArgumentsAreIgnoredUntilSeparator();
};

void MediaArgumentParserTest::noMediaArgumentsProducesEmptyResult()
{
    const ParsedMediaArguments parsed = MediaArgumentParser::parseProcessArguments(
        {QStringLiteral("Player.exe")},
        QDir::currentPath());
    QVERIFY(parsed.orderedSources.isEmpty());
}

void MediaArgumentParserTest::localPathsResolveAgainstWorkingDirectory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString relativePath = QStringLiteral("folder with spaces/媒体.mp4");
    const ParsedMediaArguments parsed = MediaArgumentParser::parseProcessArguments(
        {QStringLiteral("Player.exe"), relativePath},
        directory.path());

    QCOMPARE(parsed.orderedSources.size(), 1);
    QVERIFY(parsed.orderedSources.constFirst().isLocalFile());
    QCOMPARE(
        QDir::cleanPath(parsed.orderedSources.constFirst().toLocalFile()),
        QDir::cleanPath(QDir(directory.path()).absoluteFilePath(relativePath)));
}

void MediaArgumentParserTest::remoteUrlsRemainUrls()
{
    const ParsedMediaArguments parsed = MediaArgumentParser::parseProcessArguments(
        {QStringLiteral("Player.exe"), QStringLiteral("https://example.com/video.mp4?quality=1080")},
        QDir::currentPath());

    QCOMPARE(parsed.orderedSources.size(), 1);
    QCOMPARE(parsed.orderedSources.constFirst().scheme(), QStringLiteral("https"));
    QCOMPARE(parsed.orderedSources.constFirst().host(), QStringLiteral("example.com"));
    QCOMPARE(parsed.orderedSources.constFirst().path(), QStringLiteral("/video.mp4"));
    QCOMPARE(parsed.orderedSources.constFirst().query(), QStringLiteral("quality=1080"));
}

void MediaArgumentParserTest::multipleArgumentsPreserveOrder()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const ParsedMediaArguments parsed = MediaArgumentParser::parseProcessArguments(
        {QStringLiteral("Player.exe"),
         QStringLiteral("b.mp4"),
         QStringLiteral("https://example.com/a.mp4"),
         QStringLiteral("c.mp4")},
        directory.path());

    QCOMPARE(parsed.orderedSources.size(), 3);
    QCOMPARE(
        QDir::cleanPath(parsed.orderedSources.at(0).toLocalFile()),
        QDir::cleanPath(QDir(directory.path()).absoluteFilePath(QStringLiteral("b.mp4"))));
    QCOMPARE(parsed.orderedSources.at(1).toString(), QStringLiteral("https://example.com/a.mp4"));
    QCOMPARE(
        QDir::cleanPath(parsed.orderedSources.at(2).toLocalFile()),
        QDir::cleanPath(QDir(directory.path()).absoluteFilePath(QStringLiteral("c.mp4"))));
}

void MediaArgumentParserTest::optionLikeArgumentsAreIgnoredUntilSeparator()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const ParsedMediaArguments parsed = MediaArgumentParser::parseProcessArguments(
        {QStringLiteral("Player.exe"),
         QStringLiteral("--diagnostic"),
         QStringLiteral("normal.mp4"),
         QStringLiteral("--"),
         QStringLiteral("-leading-dash.mp4")},
        directory.path());

    QCOMPARE(parsed.orderedSources.size(), 2);
    QCOMPARE(
        QDir::cleanPath(parsed.orderedSources.at(0).toLocalFile()),
        QDir::cleanPath(QDir(directory.path()).absoluteFilePath(QStringLiteral("normal.mp4"))));
    QCOMPARE(
        QDir::cleanPath(parsed.orderedSources.at(1).toLocalFile()),
        QDir::cleanPath(QDir(directory.path()).absoluteFilePath(QStringLiteral("-leading-dash.mp4"))));
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::MediaArgumentParserTest)
#include "media_argument_parser_test.moc"
