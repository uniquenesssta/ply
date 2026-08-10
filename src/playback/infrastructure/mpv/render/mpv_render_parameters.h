#pragma once

#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <array>

namespace player::playback::infrastructure::mpv::render {

struct MpvOpenGlRenderTarget final
{
    unsigned int framebufferId = 0;
    int width = 0;
    int height = 0;
    int internalFormat = 0;
    bool flipY = false;
};

class MpvRenderParameters final
{
public:
    explicit MpvRenderParameters(MpvOpenGlRenderTarget target) noexcept;

    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] mpv_render_param* data() noexcept;
    [[nodiscard]] const mpv_render_param* data() const noexcept;

private:
    mpv_opengl_fbo framebuffer_{};
    int flipY_ = 0;
    std::array<mpv_render_param, 3> parameters_{};
    bool valid_ = false;
};

} // namespace player::playback::infrastructure::mpv::render
