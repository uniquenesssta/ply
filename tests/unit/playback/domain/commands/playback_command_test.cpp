#include "foundation/ids/request_id.h"
#include "playback/domain/commands/playback_command.h"

#include <QList>
#include <QtTest/QTest>

#include <limits>
#include <utility>

namespace player::playback::domain {
namespace {

PlaybackCommand command(player::ids::RequestId requestId, PlaybackCommandPayload payload)
{
    return PlaybackCommand{requestId, std::move(payload)};
}

} // namespace

class PlaybackCommandTest final : public QObject
{
    Q_OBJECT

private slots:
    void acceptsSupportedCommandFamilies();
    void rejectsInvalidRequestId();
    void rejectsInvalidLoadSource();
    void rejectsNonFiniteSeek();
    void rejectsInvalidVolume();
    void rejectsInvalidSpeed();
    void rejectsInvalidTrackSelection();
    void rejectsNonFiniteDelay();
    void rejectsInvalidExternalSubtitlePath();
};

void PlaybackCommandTest::acceptsSupportedCommandFamilies()
{
    const QList<PlaybackCommand> commands{
        command(player::ids::RequestId{1}, PlaybackCommandPayload{LoadMediaCommand{QStringLiteral("sample.mp4")}}),
        command(player::ids::RequestId{2}, PlaybackCommandPayload{TransportCommand{TransportAction::Play}}),
        command(player::ids::RequestId{3}, PlaybackCommandPayload{TransportCommand{TransportAction::Pause}}),
        command(player::ids::RequestId{4}, PlaybackCommandPayload{TransportCommand{TransportAction::Stop}}),
        command(player::ids::RequestId{5}, PlaybackCommandPayload{SeekCommand{12.5, SeekMode::Absolute}}),
        command(player::ids::RequestId{6}, PlaybackCommandPayload{SeekCommand{-5.0, SeekMode::Relative}}),
        command(player::ids::RequestId{7}, PlaybackCommandPayload{SetVolumeCommand{125.0}}),
        command(player::ids::RequestId{8}, PlaybackCommandPayload{SetMutedCommand{true}}),
        command(player::ids::RequestId{9}, PlaybackCommandPayload{SetSpeedCommand{1.5}}),
        command(player::ids::RequestId{10}, PlaybackCommandPayload{SelectTrackCommand{
            TrackKind::Subtitle, qint64{3}}}),
        command(player::ids::RequestId{11}, PlaybackCommandPayload{SelectTrackCommand{
            TrackKind::Audio, std::nullopt}}),
        command(player::ids::RequestId{12}, PlaybackCommandPayload{SetSubtitleDelayCommand{-0.5}}),
        command(player::ids::RequestId{13}, PlaybackCommandPayload{SetAudioDelayCommand{0.25}}),
        command(player::ids::RequestId{14}, PlaybackCommandPayload{LoadExternalSubtitleCommand{
            QStringLiteral("C:/subtitles/track.srt")}}),
        command(player::ids::RequestId{15}, PlaybackCommandPayload{LifecycleCommand{PlaybackLifecycleAction::Initialize}}),
        command(player::ids::RequestId{16}, PlaybackCommandPayload{LifecycleCommand{PlaybackLifecycleAction::Shutdown}}),
    };

    for (const PlaybackCommand& value : commands) {
        QVERIFY(value.requestId().isValid());
        QVERIFY(!validatePlaybackCommand(value).has_value());
    }
}

void PlaybackCommandTest::rejectsInvalidRequestId()
{
    const PlaybackCommand value{
        player::ids::RequestId{},
        PlaybackCommandPayload{TransportCommand{TransportAction::Play}}};

    QVERIFY(
        validatePlaybackCommand(value)
        == std::optional{PlaybackCommandValidationError::InvalidRequestId});
}

void PlaybackCommandTest::rejectsInvalidLoadSource()
{
    const PlaybackCommand empty{
        player::ids::RequestId{1},
        PlaybackCommandPayload{LoadMediaCommand{QString{}}}};
    QVERIFY(
        validatePlaybackCommand(empty)
        == std::optional{PlaybackCommandValidationError::EmptyMediaSource});

    QString embeddedNull = QStringLiteral("before");
    embeddedNull.append(QChar{u'\0'});
    embeddedNull.append(QStringLiteral("after"));
    const PlaybackCommand containsNull{
        player::ids::RequestId{2},
        PlaybackCommandPayload{LoadMediaCommand{embeddedNull}}};
    QVERIFY(
        validatePlaybackCommand(containsNull)
        == std::optional{PlaybackCommandValidationError::MediaSourceContainsNull});
}

void PlaybackCommandTest::rejectsNonFiniteSeek()
{
    const PlaybackCommand value{
        player::ids::RequestId{1},
        PlaybackCommandPayload{SeekCommand{
            std::numeric_limits<double>::infinity(),
            SeekMode::Absolute}}};

    QVERIFY(
        validatePlaybackCommand(value)
        == std::optional{PlaybackCommandValidationError::NonFiniteSeek});
}

void PlaybackCommandTest::rejectsInvalidVolume()
{
    const PlaybackCommand negative{
        player::ids::RequestId{1},
        PlaybackCommandPayload{SetVolumeCommand{-0.1}}};
    QVERIFY(
        validatePlaybackCommand(negative)
        == std::optional{PlaybackCommandValidationError::InvalidVolume});

    const PlaybackCommand notANumber{
        player::ids::RequestId{2},
        PlaybackCommandPayload{SetVolumeCommand{std::numeric_limits<double>::quiet_NaN()}}};
    QVERIFY(
        validatePlaybackCommand(notANumber)
        == std::optional{PlaybackCommandValidationError::InvalidVolume});
}

void PlaybackCommandTest::rejectsInvalidSpeed()
{
    const PlaybackCommand zero{
        player::ids::RequestId{1},
        PlaybackCommandPayload{SetSpeedCommand{0.0}}};
    QVERIFY(
        validatePlaybackCommand(zero)
        == std::optional{PlaybackCommandValidationError::InvalidSpeed});

    const PlaybackCommand notANumber{
        player::ids::RequestId{2},
        PlaybackCommandPayload{SetSpeedCommand{std::numeric_limits<double>::quiet_NaN()}}};
    QVERIFY(
        validatePlaybackCommand(notANumber)
        == std::optional{PlaybackCommandValidationError::InvalidSpeed});
}

void PlaybackCommandTest::rejectsInvalidTrackSelection()
{
    const PlaybackCommand zeroId{
        player::ids::RequestId{1},
        PlaybackCommandPayload{SelectTrackCommand{TrackKind::Audio, qint64{0}}}};
    QVERIFY(
        validatePlaybackCommand(zeroId)
        == std::optional{PlaybackCommandValidationError::InvalidTrackSelection});

    const PlaybackCommand negativeId{
        player::ids::RequestId{2},
        PlaybackCommandPayload{SelectTrackCommand{TrackKind::Subtitle, qint64{-1}}}};
    QVERIFY(
        validatePlaybackCommand(negativeId)
        == std::optional{PlaybackCommandValidationError::InvalidTrackSelection});
}

void PlaybackCommandTest::rejectsNonFiniteDelay()
{
    const PlaybackCommand subtitleNaN{
        player::ids::RequestId{1},
        PlaybackCommandPayload{SetSubtitleDelayCommand{std::numeric_limits<double>::quiet_NaN()}}};
    QVERIFY(
        validatePlaybackCommand(subtitleNaN)
        == std::optional{PlaybackCommandValidationError::NonFiniteDelay});

    const PlaybackCommand audioInfinity{
        player::ids::RequestId{2},
        PlaybackCommandPayload{SetAudioDelayCommand{std::numeric_limits<double>::infinity()}}};
    QVERIFY(
        validatePlaybackCommand(audioInfinity)
        == std::optional{PlaybackCommandValidationError::NonFiniteDelay});
}

void PlaybackCommandTest::rejectsInvalidExternalSubtitlePath()
{
    const PlaybackCommand empty{
        player::ids::RequestId{1},
        PlaybackCommandPayload{LoadExternalSubtitleCommand{QString{}}}};
    QVERIFY(
        validatePlaybackCommand(empty)
        == std::optional{PlaybackCommandValidationError::InvalidExternalSubtitlePath});

    QString embeddedNull = QStringLiteral("before");
    embeddedNull.append(QChar{u'\0'});
    embeddedNull.append(QStringLiteral("after"));
    const PlaybackCommand containsNull{
        player::ids::RequestId{2},
        PlaybackCommandPayload{LoadExternalSubtitleCommand{embeddedNull}}};
    QVERIFY(
        validatePlaybackCommand(containsNull)
        == std::optional{PlaybackCommandValidationError::InvalidExternalSubtitlePath});
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackCommandTest)
#include "playback_command_test.moc"
