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

    // Keep the libmpv-backed surface scene-graph active from application startup.
    // libmpv requires mpv_render_context_create() to complete before video
    // initialization; media capability therefore controls visual exposure, not
    // the lifetime of the render surface itself.
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
