#pragma once

#include <QPointer>

#include <cstdint>
#include <memory>
#include <thread>

class QOpenGLContext;
class QString;
struct mpv_handle;
struct mpv_render_context;
struct mpv_render_param;

namespace player::playback::infrastructure::mpv::render {

using MpvRenderUpdateCallback = void (*)(void*);

class MpvRenderContext final {
public:
    static std::unique_ptr<MpvRenderContext> create(
        mpv_handle* coreHandle,
        QString* errorMessage = nullptr);

    ~MpvRenderContext();

    MpvRenderContext(const MpvRenderContext&) = delete;
    MpvRenderContext& operator=(const MpvRenderContext&) = delete;
    MpvRenderContext(MpvRenderContext&&) = delete;
    MpvRenderContext& operator=(MpvRenderContext&&) = delete;

    [[nodiscard]] bool isOpen() const noexcept;
    [[nodiscard]] bool close(QString* errorMessage = nullptr);

    [[nodiscard]] bool setUpdateCallback(
        MpvRenderUpdateCallback callback,
        void* callbackContext,
        QString* errorMessage = nullptr);

    [[nodiscard]] bool update(
        std::uint64_t* updateFlags,
        QString* errorMessage = nullptr);

    [[nodiscard]] bool render(
        mpv_render_param* params,
        QString* errorMessage = nullptr);

private:
    MpvRenderContext(
        mpv_render_context* context,
        QOpenGLContext* openGlContext,
        std::thread::id ownerThreadId);

    [[nodiscard]] bool validateOwnerThread(QString* errorMessage) const;
    [[nodiscard]] bool validateRenderAccess(QString* errorMessage) const;

    mpv_render_context* context_ = nullptr;
    QPointer<QOpenGLContext> openGlContext_;
    std::thread::id ownerThreadId_;
};

} // namespace player::playback::infrastructure::mpv::render
