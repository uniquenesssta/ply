#pragma once

class QGuiApplication;

namespace player::app {

class ApplicationBootstrap final {
public:
    [[nodiscard]] int run(QGuiApplication& application);

private:
    static void configureApplicationMetadata(QGuiApplication& application);
};

} // namespace player::app
