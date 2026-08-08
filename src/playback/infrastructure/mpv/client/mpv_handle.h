#pragma once

#include <memory>

class QString;
struct mpv_handle;

namespace player::playback::mpv {

class MpvHandle final
{
public:
    static std::unique_ptr<MpvHandle> create(QString* errorMessage = nullptr);

    ~MpvHandle();

    MpvHandle(const MpvHandle&) = delete;
    MpvHandle& operator=(const MpvHandle&) = delete;
    MpvHandle(MpvHandle&&) = delete;
    MpvHandle& operator=(MpvHandle&&) = delete;

    [[nodiscard]] bool initialize(QString* errorMessage = nullptr);
    void close() noexcept;

    [[nodiscard]] bool isOpen() const noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] mpv_handle* nativeHandle() noexcept;
    [[nodiscard]] const mpv_handle* nativeHandle() const noexcept;

private:
    explicit MpvHandle(mpv_handle* handle) noexcept;

    mpv_handle* handle_ = nullptr;
    bool initialized_ = false;
};

} // namespace player::playback::mpv
