#include "presentation/qml/types/presentation_type_registration.h"

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QtQml/qqml.h>

namespace player::presentation::qml {

bool registerPresentationQmlTypes()
{
    static const bool registered = qmlRegisterType<
        player::playback::infrastructure::mpv::render::MpvVideoItem>(
        "Player.Presentation",
        1,
        0,
        "MpvVideoItem") >= 0;

    return registered;
}

} // namespace player::presentation::qml
