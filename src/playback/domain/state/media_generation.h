#pragma once

#include <QtGlobal>

#include <compare>

namespace player::playback::domain {

class MediaGeneration final
{
public:
    constexpr MediaGeneration() noexcept = default;
    constexpr explicit MediaGeneration(quint64 value) noexcept
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

    [[nodiscard]] constexpr auto operator<=>(const MediaGeneration&) const noexcept = default;

private:
    quint64 value_ = 0;
};

} // namespace player::playback::domain
