#include "playback/infrastructure/mpv/runtime/mpv_runtime_probe.h"

#include <QFileInfo>
#include <QtTest>

namespace player::playback::mpv {

class MpvRuntimeProbeTest final : public QObject
{
    Q_OBJECT

private slots:
    void validatesStagedRuntimeAndManifest();
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

} // namespace player::playback::mpv

QTEST_GUILESS_MAIN(player::playback::mpv::MpvRuntimeProbeTest)

#include "mpv_runtime_probe_test.moc"
