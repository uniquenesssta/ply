#pragma once

#include "media/application/open/media_open_error.h"
#include "media/application/open/media_open_operation_id.h"
#include "media/domain/media_source.h"

#include <QList>
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
    using SubmitMediaBatch =
        std::function<bool(const QList<player::media::domain::MediaSource>&)>;

    explicit MediaOpenCoordinator(SubmitMedia submitMedia, QObject* parent = nullptr);
    MediaOpenCoordinator(
        SubmitMedia submitMedia,
        SubmitMediaBatch submitMediaBatch,
        QObject* parent = nullptr);

    [[nodiscard]] QString lastErrorKey() const;

    [[nodiscard]] MediaOpenOperationId beginReplaceOpenOperation() noexcept;
    [[nodiscard]] bool isOpenOperationCurrent(MediaOpenOperationId operationId) const noexcept;
    [[nodiscard]] bool cancelOpenOperation(MediaOpenOperationId operationId) noexcept;
    [[nodiscard]] bool acceptingOpenOperations() const noexcept;
    void beginShutdown() noexcept;

    [[nodiscard]] bool completeOpenSource(
        MediaOpenOperationId operationId,
        const player::media::domain::MediaSource& source);
    [[nodiscard]] bool completeOpenSources(
        MediaOpenOperationId operationId,
        const QList<player::media::domain::MediaSource>& sources);

    [[nodiscard]] bool openSource(const player::media::domain::MediaSource& source);
    [[nodiscard]] bool openSources(
        const QList<player::media::domain::MediaSource>& sources);
    [[nodiscard]] bool openSourceUrls(const QList<QUrl>& sourceUrls);
    Q_INVOKABLE bool openLocalFile(const QUrl& sourceUrl);
    Q_INVOKABLE bool openLocalFiles(const QList<QUrl>& sourceUrls);

signals:
    void lastErrorKeyChanged();
    void openRejected(const QString& errorKey);

private:
    [[nodiscard]] bool consumeOpenOperation(MediaOpenOperationId operationId) noexcept;
    [[nodiscard]] bool rejectOpenOperation(
        MediaOpenOperationId operationId,
        MediaOpenError error);
    [[nodiscard]] bool submitSource(
        const player::media::domain::MediaSource& source);
    [[nodiscard]] bool submitSources(
        const QList<player::media::domain::MediaSource>& sources);
    void setError(MediaOpenError error);

    SubmitMedia submitMedia_;
    SubmitMediaBatch submitMediaBatch_;
    MediaOpenError lastError_ = MediaOpenError::None;
    quint64 nextOperationValue_ = 1;
    MediaOpenOperationId currentOperation_;
    bool acceptingOpenOperations_ = true;
};

} // namespace player::media::application
