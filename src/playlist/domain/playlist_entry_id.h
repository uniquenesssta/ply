#pragma once

#include <QtGlobal>

namespace player::playlist::domain {

class PlaylistEntryId final
{
public:
    constexpr PlaylistEntryId() noexcept = default;
    explicit constexpr PlaylistEntryId(quint64 value) noexcept
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

    friend constexpr bool operator==(
        PlaylistEntryId lhs,
        PlaylistEntryId rhs) noexcept = default;

private:
    quint64 value_ = 0;
};

} // namespace player::playlist::domain
