#pragma once

#include "app/bootstrap/runtime_paths.h"

#include <memory>

class QGuiApplication;

namespace player::app {

class LoggingBootstrap;

class ApplicationBootstrap final {
public:
    static void configureApplicationMetadata();

    [[nodiscard]] int run(
        QGuiApplication& application,
        RuntimePaths runtimePaths,
        std::unique_ptr<LoggingBootstrap> loggingBootstrap);
};

} // namespace player::app
