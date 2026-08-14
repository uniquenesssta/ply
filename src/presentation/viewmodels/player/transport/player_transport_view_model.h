#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>

namespace player::presentation {

class PlayerTransportViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canPlay READ canPlay NOTIFY stateChanged)
    Q_PROPERTY(bool canPause READ canPause NOTIFY stateChanged)
    Q_PROPERTY(bool canStop READ canStop NOTIFY stateChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY stateChanged)
    Q_PROPERTY(bool canPrevious READ canPrevious CONSTANT)
    Q_PROPERTY(bool canNext READ canNext CONSTANT)

public:
    explicit PlayerTransportViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool canPlay() const noexcept;
    [[nodiscard]] bool canPause() const noexcept;
    [[nodiscard]] bool canStop() const noexcept;
    [[nodiscard]] bool isPlaying() const noexcept;
    [[nodiscard]] bool canPrevious() const noexcept;
    [[nodiscard]] bool canNext() const noexcept;

    Q_INVOKABLE bool requestPlay();
    Q_INVOKABLE bool requestPause();
    Q_INVOKABLE bool requestTogglePlayPause();
    Q_INVOKABLE bool requestStop();
    Q_INVOKABLE bool requestPrevious();
    Q_INVOKABLE bool requestNext();

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();
    void playRequested();
    void pauseRequested();
    void stopRequested();

private:
    bool canPlay_ = false;
    bool canPause_ = false;
    bool canStop_ = false;
    bool isPlaying_ = false;
};

} // namespace player::presentation
