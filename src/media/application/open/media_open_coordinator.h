#pragma once

#include "media/application/open/media_open_error.h"
#include "media/domain/media_source.h"

#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

namespace player::media::application {

class MediaOpenCoordinator final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastErrorKey READ lastErrorKey NOTIFY lastErrorKeyChanged)

public:
    using SubmitMedia = std::function<bool(const player::media::domain::MediaSource&)>;

    explicit MediaOpenCoordinator(SubmitMedia submitMedia, QObject* parent = nullptr);

    [[nodiscard]] QString lastErrorKey() const;

    Q_INVOKABLE bool openLocalFile(const QUrl& sourceUrl);

signals:
    void lastErrorKeyChanged();
    void openRejected(const QString& errorKey);

private:
    void setError(MediaOpenError error);

    SubmitMedia submitMedia_;
    MediaOpenError lastError_ = MediaOpenError::None;
};

} // namespace player::media::application
