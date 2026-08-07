#pragma once

#include <memory>

class QString;

namespace player::logging {
class LogFileSink;
}

namespace player::app {

class RuntimePaths;

class LoggingBootstrap final
{
public:
    LoggingBootstrap();
    ~LoggingBootstrap();

    LoggingBootstrap(const LoggingBootstrap&) = delete;
    LoggingBootstrap& operator=(const LoggingBootstrap&) = delete;

    [[nodiscard]] bool start(const RuntimePaths& paths, QString* errorMessage = nullptr);
    void stop();

private:
    std::unique_ptr<player::logging::LogFileSink> m_sink;
};

} // namespace player::app
