import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    objectName: "playerVideoViewport"
    z: ZOrderTokens.video
    clip: true

    VideoSurface {
        id: videoSurface
        objectName: "playerVideoSurface"
        anchors.fill: parent
    }
}
