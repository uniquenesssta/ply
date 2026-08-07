#pragma once

#include <QQmlApplicationEngine>
#include <QString>
#include <QStringList>

namespace player::app {

class QmlBootstrap final
{
public:
    QmlBootstrap();

    [[nodiscard]] bool load();
    [[nodiscard]] const QString& lastError() const noexcept;

private:
    QQmlApplicationEngine engine_;
    QStringList warningMessages_;
    QString lastError_;
};

} // namespace player::app
