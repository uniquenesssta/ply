#pragma once

#include "playback/domain/state/playback_snapshot.h"
#include "presentation/viewmodels/player/timeline/timeline_scrub_session.h"

#include <QObject>
#include <QString>

#include <optional>

namespace player::presentation {

class PlayerTimelineViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY stateChanged)
    Q_PROPERTY(bool isScrubbing READ isScrubbing NOTIFY stateChanged)
    Q_PROPERTY(bool seekPending READ seekPending NOTIFY stateChanged)
    Q_PROPERTY(bool backendSeeking READ backendSeeking NOTIFY stateChanged)
    Q_PROPERTY(double displayedNormalized READ displayedNormalized NOTIFY stateChanged)
    Q_PROPERTY(double durationSeconds READ durationSeconds NOTIFY stateChanged)
    Q_PROPERTY(QString positionText READ positionText NOTIFY stateChanged)
    Q_PROPERTY(QString durationText READ durationText NOTIFY stateChanged)

public:
    explicit PlayerTimelineViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool canSeek() const noexcept;
    [[nodiscard]] bool isScrubbing() const noexcept;
    [[nodiscard]] bool seekPending() const noexcept;
    [[nodiscard]] bool backendSeeking() const noexcept;
    [[nodiscard]] double displayedNormalized() const noexcept;
    [[nodiscard]] double durationSeconds() const noexcept;
    [[nodiscard]] QString positionText() const;
    [[nodiscard]] QString durationText() const;

    Q_INVOKABLE bool beginScrub(double normalized);
    Q_INVOKABLE bool updateScrub(double normalized);
    Q_INVOKABLE bool commitScrub(double normalized);
    Q_INVOKABLE bool cancelScrub();

    [[nodiscard]] bool rejectPendingSeek();

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();
    void seekRequested(double absoluteSeconds);

private:
    [[nodiscard]] double displayedPositionSeconds() const noexcept;
    [[nodiscard]] static QString formatTimecode(double seconds);
    [[nodiscard]] static std::optional<double> sanitizePosition(
        const std::optional<double>& seconds) noexcept;
    [[nodiscard]] static std::optional<double> sanitizeDuration(
        const std::optional<double>& seconds) noexcept;

    player::playback::domain::MediaGeneration generation_;
    std::optional<double> actualPositionSeconds_;
    std::optional<double> durationSeconds_;
    bool canSeek_ = false;
    bool backendSeeking_ = false;
    bool pendingSawBackendSeeking_ = false;
    TimelineScrubSession scrubSession_;
};

} // namespace player::presentation
