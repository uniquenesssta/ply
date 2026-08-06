#pragma once

#include <QQmlApplicationEngine>

namespace player::app {

class QmlBootstrap final {
public:
    [[nodiscard]] bool load();

private:
    QQmlApplicationEngine engine_;
};

} // namespace player::app
