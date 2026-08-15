#pragma once

#include "playback/domain/state/playback_snapshot.h"
#include "presentation/viewmodels/player/status/player_status_selector.h"

#include <QObject>
#include <QString>

namespace player::presentation {

class PlayerStatusViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusKey READ statusKey NOTIFY stateChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY stateChanged)
    Q_PROPERTY(bool errorVisible READ errorVisible NOTIFY stateChanged)
    Q_PROPERTY(int bufferingPercent READ bufferingPercent NOTIFY stateChanged)

public:
    explicit PlayerStatusViewModel(QObject* parent = nullptr);

    [[nodiscard]] PlayerStatusKind status() const noexcept;
    [[nodiscard]] QString statusKey() const;
    [[nodiscard]] bool visible() const noexcept;
    [[nodiscard]] bool errorVisible() const noexcept;
    [[nodiscard]] int bufferingPercent() const noexcept;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();

private:
    PlayerStatusKind status_ = PlayerStatusKind::None;
    int bufferingPercent_ = -1;
};

} // namespace player::presentation
