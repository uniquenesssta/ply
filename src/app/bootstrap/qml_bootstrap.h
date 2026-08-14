#pragma once

#include <QQmlApplicationEngine>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace player::app {

class QmlBootstrap final
{
public:
    QmlBootstrap();

    void setInitialProperties(const QVariantMap& properties);
    [[nodiscard]] bool load();
    [[nodiscard]] const QString& lastError() const noexcept;

private:
    QQmlApplicationEngine engine_;
    QStringList warningMessages_;
    QString lastError_;
};

} // namespace player::app
