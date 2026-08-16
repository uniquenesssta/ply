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

    // Keep the render surface scene-graph active from application startup.
    // The render context must be ready before video initialization; media
    // capability therefore controls visual exposure, not render-surface lifetime.
    VideoSurface {
        id: videoSurface
        objectName: "playerVideoSurface"
        anchors.fill: parent
    }

    // Empty/audio states remain visually unchanged while the render surface is
    // prewarmed underneath. The cover owns no input handlers and disappears as
    // soon as the current media is known to contain a video track.
    Rectangle {
        objectName: "playerVideoViewportBackground"
        anchors.fill: parent
        visible: !root.videoVisible
        color: root.backgroundColor
    }
}
