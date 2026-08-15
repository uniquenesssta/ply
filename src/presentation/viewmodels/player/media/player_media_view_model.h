#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>

namespace player::presentation {

class PlayerMediaViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasMedia READ hasMedia NOTIFY stateChanged)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY stateChanged)

public:
    explicit PlayerMediaViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool hasMedia() const noexcept;
    [[nodiscard]] bool hasVideo() const noexcept;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();

private:
    bool hasMedia_ = false;
    bool hasVideo_ = false;
};

} // namespace player::presentation
