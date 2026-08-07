#pragma once

#include <QString>

namespace player::logging {

class LogRedactor final
{
public:
    [[nodiscard]] static QString redact(QString text);
};

} // namespace player::logging
