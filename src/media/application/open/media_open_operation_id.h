#pragma once

#include <QtGlobal>

namespace player::media::application {

class MediaOpenOperationId final
{
public:
    constexpr MediaOpenOperationId() noexcept = default;
    explicit constexpr MediaOpenOperationId(quint64 value) noexcept
        : value_(value)
    {
    }

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return value_ != 0;
    }

    [[nodiscard]] constexpr quint64 value() const noexcept
    {
        return value_;
    }

    friend constexpr bool operator==(
        MediaOpenOperationId lhs,
        MediaOpenOperationId rhs) noexcept = default;

private:
    quint64 value_ = 0;
};

} // namespace player::media::application
