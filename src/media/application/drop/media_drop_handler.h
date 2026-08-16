#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>

namespace player::media::application {

class MediaOpenCoordinator;

class MediaDropHandler final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastOutcomeKey READ lastOutcomeKey NOTIFY dropResultChanged)
    Q_PROPERTY(QVariantList orderedSourceUrls READ orderedSourceUrls NOTIFY dropResultChanged)

public:
    explicit MediaDropHandler(MediaOpenCoordinator& mediaOpenCoordinator, QObject* parent = nullptr);

    [[nodiscard]] QString lastOutcomeKey() const;
    [[nodiscard]] QVariantList orderedSourceUrls() const;

    Q_INVOKABLE bool canHandle(const QList<QUrl>& sourceUrls) const;
    Q_INVOKABLE bool handleDrop(const QList<QUrl>& sourceUrls);

signals:
    void dropResultChanged();
    void dropDeferred(const QString& outcomeKey, const QVariantList& orderedSourceUrls);
    void dropRejected(const QString& outcomeKey);

private:
    enum class Classification {
        Empty,
        SingleLocalFile,
        MultipleLocalFiles,
        RemoteUrl,
        Directory,
        Unsupported,
    };

    [[nodiscard]] Classification classify(const QList<QUrl>& sourceUrls) const;
    void setResult(const QString& outcomeKey, const QList<QUrl>& sourceUrls);

    MediaOpenCoordinator& mediaOpenCoordinator_;
    QString lastOutcomeKey_;
    QVariantList orderedSourceUrls_;
};

} // namespace player::media::application
