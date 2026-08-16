#pragma once

#include "media/application/open/media_open_error.h"

#include <QObject>
#include <QString>

namespace player::media::application {

class MediaOpenCoordinator;

class UrlOpenWorkflow final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastErrorKey READ lastErrorKey NOTIFY lastErrorKeyChanged)

public:
    explicit UrlOpenWorkflow(MediaOpenCoordinator& mediaOpenCoordinator, QObject* parent = nullptr);

    [[nodiscard]] QString lastErrorKey() const;
    Q_INVOKABLE bool openUrl(const QString& sourceText);

signals:
    void lastErrorKeyChanged();
    void openRejected(const QString& errorKey);

private:
    void setError(MediaOpenError error);

    MediaOpenCoordinator& mediaOpenCoordinator_;
    MediaOpenError lastError_ = MediaOpenError::None;
};

} // namespace player::media::application
