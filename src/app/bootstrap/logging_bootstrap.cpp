#include "app/bootstrap/logging_bootstrap.h"

#include "app/bootstrap/runtime_paths.h"
#include "foundation/logging/log_file_sink.h"

#include <QDebug>
#include <QString>

#include <utility>

namespace player::app {

LoggingBootstrap::LoggingBootstrap() = default;

LoggingBootstrap::~LoggingBootstrap()
{
    stop();
}

bool LoggingBootstrap::start(const RuntimePaths& paths, QString* errorMessage)
{
    if (m_sink != nullptr && m_sink->isActive()) {
        return true;
    }

    if (m_sink != nullptr) {
        m_sink->stop();
        m_sink.reset();
    }

    player::logging::LogFileSinkOptions options;
    options.directory = paths.logDirectory();

    auto sink = std::make_unique<player::logging::LogFileSink>(std::move(options));
    if (!sink->start(errorMessage)) {
        return false;
    }

    m_sink = std::move(sink);
    qInfo().noquote() << "File logging active:" << m_sink->currentLogFilePath();
    return true;
}

void LoggingBootstrap::stop()
{
    if (m_sink != nullptr) {
        m_sink->stop();
        m_sink.reset();
    }
}

} // namespace player::app
