#include "foundation/ids/request_id.h"
#include "playback/domain/commands/playback_command.h"

#include <QList>
#include <QtTest/QTest>

#include <limits>
#include <optional>
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
    void rejectsInvalidExternalSubtitleSource();
    void rejectsNonFiniteSeek();
    void rejectsInvalidVolume();
    void rejectsInvalidSpeed();
    void rejectsInvalidSubtitleDelay();
    void rejectsInvalidTrackSelection();
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
        command(player::ids::RequestId{10}, PlaybackCommandPayload{AddExternalSubtitleCommand{QStringLiteral("captions.srt")}}),
        command(player::ids::RequestId{11}, PlaybackCommandPayload{TrackSelectionCommand{TrackSelectionKind::Audio, qint64{2}}}),
        command(player::ids::RequestId{12}, PlaybackCommandPayload{TrackSelectionCommand{TrackSelectionKind::Subtitle, qint64{7}}}),
        command(player::ids::RequestId{13}, PlaybackCommandPayload{TrackSelectionCommand{TrackSelectionKind::Subtitle, std::nullopt}}),
        command(player::ids::RequestId{14}, PlaybackCommandPayload{LifecycleCommand{PlaybackLifecycleAction::Initialize}}),
        command(player::ids::RequestId{15}, PlaybackCommandPayload{LifecycleCommand{PlaybackLifecycleAction::Shutdown}}),
        command(player::ids::RequestId{16}, PlaybackCommandPayload{SetSubtitleDelayCommand{0.25}}),
        command(player::ids::RequestId{17}, PlaybackCommandPayload{SetSubtitleDelayCommand{-0.25}}),
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

void PlaybackCommandTest::rejectsInvalidExternalSubtitleSource()
{
    const PlaybackCommand empty{
        player::ids::RequestId{1},
        PlaybackCommandPayload{AddExternalSubtitleCommand{QString{}}}};
    QVERIFY(
        validatePlaybackCommand(empty)
        == std::optional{PlaybackCommandValidationError::EmptyExternalSubtitleSource});

    QString embeddedNull = QStringLiteral("captions");
    embeddedNull.append(QChar{u'\0'});
    embeddedNull.append(QStringLiteral(".srt"));
    const PlaybackCommand containsNull{
        player::ids::RequestId{2},
        PlaybackCommandPayload{AddExternalSubtitleCommand{embeddedNull}}};
    QVERIFY(
        validatePlaybackCommand(containsNull)
        == std::optional{PlaybackCommandValidationError::ExternalSubtitleSourceContainsNull});
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

void PlaybackCommandTest::rejectsInvalidSubtitleDelay()
{
    const PlaybackCommand tooHigh{
        player::ids::RequestId{1},
        PlaybackCommandPayload{SetSubtitleDelayCommand{2.01}}};
    QVERIFY(
        validatePlaybackCommand(tooHigh)
        == std::optional{PlaybackCommandValidationError::InvalidSubtitleDelay});

    const PlaybackCommand tooLow{
        player::ids::RequestId{2},
        PlaybackCommandPayload{SetSubtitleDelayCommand{-2.01}}};
    QVERIFY(
        validatePlaybackCommand(tooLow)
        == std::optional{PlaybackCommandValidationError::InvalidSubtitleDelay});

    const PlaybackCommand notANumber{
        player::ids::RequestId{3},
        PlaybackCommandPayload{SetSubtitleDelayCommand{std::numeric_limits<double>::quiet_NaN()}}};
    QVERIFY(
        validatePlaybackCommand(notANumber)
        == std::optional{PlaybackCommandValidationError::InvalidSubtitleDelay});
}

void PlaybackCommandTest::rejectsInvalidTrackSelection()
{
    const PlaybackCommand zeroTrack{
        player::ids::RequestId{1},
        PlaybackCommandPayload{TrackSelectionCommand{TrackSelectionKind::Audio, qint64{0}}}};
    QVERIFY(
        validatePlaybackCommand(zeroTrack)
        == std::optional{PlaybackCommandValidationError::InvalidTrackId});

    const PlaybackCommand negativeTrack{
        player::ids::RequestId{2},
        PlaybackCommandPayload{TrackSelectionCommand{TrackSelectionKind::Subtitle, qint64{-1}}}};
    QVERIFY(
        validatePlaybackCommand(negativeTrack)
        == std::optional{PlaybackCommandValidationError::InvalidTrackId});

    const PlaybackCommand audioOff{
        player::ids::RequestId{3},
        PlaybackCommandPayload{TrackSelectionCommand{TrackSelectionKind::Audio, std::nullopt}}};
    QVERIFY(
        validatePlaybackCommand(audioOff)
        == std::optional{PlaybackCommandValidationError::AudioTrackCannotBeDisabled});
}

} // namespace player::playback::domain

QTEST_GUILESS_MAIN(player::playback::domain::PlaybackCommandTest)
#include "playback_command_test.moc"
