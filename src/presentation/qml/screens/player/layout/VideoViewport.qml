import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property bool hasMedia: false
    property bool hasVideo: false

    readonly property bool videoVisible: root.hasMedia && root.hasVideo
    readonly property bool audioOnly: root.hasMedia && !root.hasVideo
    readonly property color backgroundColor: root.videoVisible
        ? ColorTokens.surfaceLetterbox
        : root.audioOnly
            ? ColorTokens.surfaceAudio
            : ColorTokens.surfaceEmpty

    objectName: "playerVideoViewport"
    z: ZOrderTokens.video
    clip: true

    Rectangle {
        objectName: "playerVideoViewportBackground"
        anchors.fill: parent
        color: root.backgroundColor
    }

    VideoSurface {
        id: videoSurface
        objectName: "playerVideoSurface"
        anchors.fill: parent
        visible: root.videoVisible
    }
}
