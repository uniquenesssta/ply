#pragma once

#include "playback/domain/state/playback_snapshot.h"

#include <QObject>
#include <QString>

namespace player::presentation {

class PlayerMediaViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasMedia READ hasMedia NOTIFY stateChanged)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY stateChanged)
    Q_PROPERTY(QString title READ title NOTIFY stateChanged)
    Q_PROPERTY(QString metadataText READ metadataText NOTIFY stateChanged)

public:
    explicit PlayerMediaViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool hasMedia() const noexcept;
    [[nodiscard]] bool hasVideo() const noexcept;
    [[nodiscard]] QString title() const;
    [[nodiscard]] QString metadataText() const;

public slots:
    void acceptSnapshot(const player::playback::domain::PlaybackSnapshot& snapshot);

signals:
    void stateChanged();

private:
    bool hasMedia_ = false;
    bool hasVideo_ = false;
    QString title_;
    QString metadataText_;
};

} // namespace player::presentation
