#pragma once

class QString;

namespace player::playback::mpv {

class MpvHandle;
class MpvOptionProfile;

class MpvInitializer final
{
public:
    [[nodiscard]] static bool initializeProduct(
        MpvHandle& handle,
        QString* errorMessage = nullptr);

    [[nodiscard]] static bool initialize(
        MpvHandle& handle,
        const MpvOptionProfile& profile,
        QString* errorMessage = nullptr);
};

} // namespace player::playback::mpv
