#pragma once

#include "playback/domain/commands/external_subtitle_command.h"

#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

namespace player::tracks::application {

class ExternalSubtitleLoader final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastErrorKey READ lastErrorKey NOTIFY lastErrorKeyChanged)

public:
    using SubmitExternalSubtitle = std::function<bool(
        const player::playback::domain::AddExternalSubtitleCommand&)>;

    explicit ExternalSubtitleLoader(
        SubmitExternalSubtitle submitExternalSubtitle,
        QObject* parent = nullptr);

    [[nodiscard]] QString lastErrorKey() const;

    Q_INVOKABLE bool loadLocalSubtitle(const QUrl& sourceUrl);

signals:
    void lastErrorKeyChanged();

private:
    [[nodiscard]] bool reject(QString errorKey);
    void setLastErrorKey(QString errorKey);

    SubmitExternalSubtitle submitExternalSubtitle_;
    QString lastErrorKey_;
};

} // namespace player::tracks::application
