#include "media/application/open/url_media_validator.h"

#include <QtTest>

#include <variant>

namespace player::media::application {

class UrlMediaValidatorTest final : public QObject
{
    Q_OBJECT

private slots:
    void rejectsEmptyInput();
    void rejectsMalformedOrRelativeUrl();
    void rejectsUnsupportedScheme();
    void acceptsHttpAndHttpsUrls_data();
    void acceptsHttpAndHttpsUrls();
};

void UrlMediaValidatorTest::rejectsEmptyInput()
{
    const UrlMediaValidationResult result = UrlMediaValidator::validate(QStringLiteral("   "));
    const auto* error = std::get_if<MediaOpenError>(&result);
    QVERIFY(error != nullptr);
    QCOMPARE(
        static_cast<int>(*error),
        static_cast<int>(MediaOpenError::EmptyUrl));
}

void UrlMediaValidatorTest::rejectsMalformedOrRelativeUrl()
{
    for (const QString& input : {
             QStringLiteral("video.mp4"),
             QStringLiteral("https:///missing-host.mp4"),
             QStringLiteral("not a url")}) {
        const UrlMediaValidationResult result = UrlMediaValidator::validate(input);
        const auto* error = std::get_if<MediaOpenError>(&result);
        QVERIFY2(error != nullptr, qPrintable(input));
        QCOMPARE(
            static_cast<int>(*error),
            static_cast<int>(MediaOpenError::InvalidUrl));
    }
}

void UrlMediaValidatorTest::rejectsUnsupportedScheme()
{
    const UrlMediaValidationResult result = UrlMediaValidator::validate(
        QStringLiteral("ftp://example.com/video.mp4"));
    const auto* error = std::get_if<MediaOpenError>(&result);
    QVERIFY(error != nullptr);
    QCOMPARE(
        static_cast<int>(*error),
        static_cast<int>(MediaOpenError::UnsupportedUrlScheme));
}

void UrlMediaValidatorTest::acceptsHttpAndHttpsUrls_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("http")
        << QStringLiteral("http://example.com/video.mp4")
        << QStringLiteral("http://example.com/video.mp4");
    QTest::newRow("https-trim-and-normalize-scheme")
        << QStringLiteral("  HTTPS://example.com/video.mp4?token=abc#clip  ")
        << QStringLiteral("https://example.com/video.mp4?token=abc#clip");
    QTest::newRow("encoded-path")
        << QStringLiteral("https://example.com/media%20folder/video.mp4")
        << QStringLiteral("https://example.com/media%20folder/video.mp4");
}

void UrlMediaValidatorTest::acceptsHttpAndHttpsUrls()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    const UrlMediaValidationResult result = UrlMediaValidator::validate(input);
    const auto* source = std::get_if<player::media::domain::MediaSource>(&result);
    QVERIFY(source != nullptr);
    QCOMPARE(
        static_cast<int>(source->kind()),
        static_cast<int>(player::media::domain::MediaSourceKind::RemoteUrl));
    QCOMPARE(source->location(), expected);
    QVERIFY(source->isValid());
}

} // namespace player::media::application

QTEST_GUILESS_MAIN(player::media::application::UrlMediaValidatorTest)
#include "url_media_validator_test.moc"
