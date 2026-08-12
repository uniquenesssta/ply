#pragma once

#include "playback/infrastructure/mpv/render/mpv_video_item.h"

#include <QtQmlIntegration/qqmlintegration.h>

namespace player::presentation::qml {

struct MpvVideoItemQmlType final
{
    Q_GADGET
    QML_FOREIGN(player::playback::infrastructure::mpv::render::MpvVideoItem)
    QML_NAMED_ELEMENT(MpvVideoItem)
};

} // namespace player::presentation::qml
