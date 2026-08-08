#pragma once

#include <QtGlobal>

namespace player::ids {

class RequestId final
{
public:
    constexpr RequestId() noexcept = default;
    constexpr explicit RequestId(quint64 value) noexcept
        : value_(value)
    {
    }

    [[nodiscard]] constexpr quint64 value() const noexcept
    {
        return value_;
    }

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return value_ != 0;
    }

    [[nodiscard]] constexpr bool operator==(const RequestId&) const noexcept = default;

private:
    quint64 value_ = 0;
};

} // namespace player::ids
