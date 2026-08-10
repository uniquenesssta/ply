#include "mpv_render_parameters.h"

#include <limits>

namespace player::playback::infrastructure::mpv::render {

MpvRenderParameters::MpvRenderParameters(MpvOpenGlRenderTarget target) noexcept
{
    if (target.width <= 0 || target.height <= 0
        || target.framebufferId > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
        return;
    }

    framebuffer_ = mpv_opengl_fbo{
        static_cast<int>(target.framebufferId),
        target.width,
        target.height,
        target.internalFormat,
    };
    flipY_ = target.flipY ? 1 : 0;
    parameters_ = {
        mpv_render_param{MPV_RENDER_PARAM_OPENGL_FBO, &framebuffer_},
        mpv_render_param{MPV_RENDER_PARAM_FLIP_Y, &flipY_},
        mpv_render_param{MPV_RENDER_PARAM_INVALID, nullptr},
    };
    valid_ = true;
}

bool MpvRenderParameters::isValid() const noexcept
{
    return valid_;
}

mpv_render_param* MpvRenderParameters::data() noexcept
{
    return parameters_.data();
}

const mpv_render_param* MpvRenderParameters::data() const noexcept
{
    return parameters_.data();
}

} // namespace player::playback::infrastructure::mpv::render
