#pragma once

#include <QString>

namespace player::app {

struct GraphicsBackendInfo final {
    QString vendor;
    QString renderer;
    QString version;
    QString shadingLanguageVersion;
    bool isOpenGles = false;
    int majorVersion = 0;
    int minorVersion = 0;
};

class GraphicsBackendProbe final {
public:
    static bool probe(GraphicsBackendInfo& info, QString* errorMessage = nullptr);
};

} // namespace player::app
