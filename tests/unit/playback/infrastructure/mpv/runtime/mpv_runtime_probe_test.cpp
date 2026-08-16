#include "playback/infrastructure/mpv/runtime/mpv_runtime_probe.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest>

namespace player::playback::mpv {

class MpvRuntimeProbeTest final : public QObject
{
    Q_OBJECT

private slots:
    void validatesStagedRuntimeAndManifest();
    void manifestRequiresWindowsTlsForHttpsMedia();
};

void MpvRuntimeProbeTest::validatesStagedRuntimeAndManifest()
{
    MpvRuntimeInfo info;
    QString errorMessage;

    QVERIFY2(MpvRuntimeProbe::probe(info, &errorMessage), qPrintable(errorMessage));
    QVERIFY(QFileInfo::exists(info.runtimeLibraryPath));
    QVERIFY(QFileInfo::exists(info.manifestPath));
    QCOMPARE(info.manifest.mpvVersion, QStringLiteral(PLAYER_EXPECTED_MPV_VERSION));
    QCOMPARE(info.manifest.mpvTag, QStringLiteral(PLAYER_EXPECTED_MPV_TAG));
    QCOMPARE(info.manifest.mpvCommit, QStringLiteral(PLAYER_EXPECTED_MPV_COMMIT));
    QCOMPARE(info.manifest.ffmpegVersion, QStringLiteral(PLAYER_EXPECTED_FFMPEG_VERSION));
    QVERIFY(info.clientApiMajor > 0);
}

void MpvRuntimeProbeTest::manifestRequiresWindowsTlsForHttpsMedia()
{
    MpvRuntimeInfo info;
    QString errorMessage;
    QVERIFY2(MpvRuntimeProbe::probe(info, &errorMessage), qPrintable(errorMessage));

    QFile manifestFile(info.manifestPath);
    QVERIFY2(manifestFile.open(QIODevice::ReadOnly), qPrintable(manifestFile.errorString()));

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(manifestFile.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QVERIFY(document.isObject());

    const QJsonArray ffmpegPolicy = document.object()
                                         .value(QStringLiteral("buildPolicy"))
                                         .toObject()
                                         .value(QStringLiteral("ffmpeg"))
                                         .toArray();
    QVERIFY2(
        ffmpegPolicy.contains(QStringLiteral("enable-schannel")),
        "The audited Windows FFmpeg package must explicitly enable Schannel when autodetect is disabled so HTTPS media is supported.");
}

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvRuntimeProbeTest)

#include "mpv_runtime_probe_test.moc"
