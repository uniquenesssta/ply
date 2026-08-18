import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property string mediaTitle: ""
    property string mediaMetadataText: ""
    property bool windowExpanded: false
    property bool windowActive: true
    property bool fullScreen: false
    property bool popupOpen: false
    property bool menuOpen: false
    property bool drawerOpen: false
    property bool modalActive: false
    property bool dragActive: false
    property bool cursorHideSuppressed: false
    property var transportViewModel: null
    property var timelineViewModel: null
    property var volumeViewModel: null
    property var statusViewModel: null
    property var mediaViewModel: null
    property var hudMessageQueue: null
    property var mediaDropHandler: null
    property var playlistController: null
    property var playlistModel: null
    property var audioTrackModel: null
    property var subtitleTrackModel: null
    property bool playlistDrawerOpen: false

    readonly property bool headerCompact: root.width < LayoutTokens.windowMinimumWidth
    readonly property bool oscCompact: root.fullScreen
    readonly property string resolvedMediaTitle: root.mediaTitle.trim().length > 0
                                                  ? root.mediaTitle
                                                  : root.mediaViewModel !== null
                                                    ? root.mediaViewModel.title
                                                    : ""
    readonly property string resolvedMediaMetadataText: root.mediaMetadataText.trim().length > 0
                                                         ? root.mediaMetadataText
                                                         : root.mediaViewModel !== null
                                                           ? root.mediaViewModel.metadataText
                                                           : ""
    readonly property string playbackStatusText: root.resolvedMediaTitle.trim().length === 0
                                                  || root.transportViewModel === null
                                                  ? ""
                                                  : root.transportViewModel.isPlaying
                                                    ? qsTr("PLAYING")
                                                    : root.transportViewModel.canPlay
                                                      ? qsTr("PAUSED")
                                                      : ""
    readonly property bool playbackPlaying: root.transportViewModel !== null
                                            && root.transportViewModel.isPlaying
    readonly property bool timelineInteractionActive: root.timelineViewModel !== null
                                                      && (root.timelineViewModel.isScrubbing
                                                          || root.timelineViewModel.seekPending)
    readonly property bool controlsDragActive: root.dragActive
                                               || dropOverlay.dragActive
                                               || volumeControls.interactionActive
    readonly property bool errorOverlayVisible: root.statusViewModel !== null
                                                && root.statusViewModel.errorVisible
    readonly property bool chromeControlsHovered: playerOscLayout.controlsHovered
    readonly property bool chromeControlsFocused: topRegion.controlsFocused
                                                  || playerOscLayout.controlsFocused
    readonly property bool pointerInsideWindow: chromeActivityLayer.pointerInside
    readonly property bool oscVisible: chromeVisibilityController.chromeVisible
    readonly property bool cursorHidden: cursorVisibilityController.cursorHidden
    readonly property bool anyDrawerOpen: root.drawerOpen || root.playlistDrawerOpen

    signal openMediaRequested()
    signal openUrlRequested()
    signal minimizeRequested()
    signal maximizeRestoreRequested()
    signal fullscreenToggleRequested()
    signal closeRequested()

    objectName: "playerScreen"

    VideoViewport {
        id: videoViewport
        anchors.fill: parent
        hasMedia: root.mediaViewModel !== null && root.mediaViewModel.hasMedia
        hasVideo: root.mediaViewModel !== null && root.mediaViewModel.hasVideo
    }

    FullscreenGestureLayer {
        id: fullscreenGestureLayer

        parent: videoViewport
        anchors.fill: parent

        onToggleFullscreenRequested: root.fullscreenToggleRequested()
    }

    PlayerChromeVisibilityController {
        id: chromeVisibilityController

        playing: root.playbackPlaying
        scrubbing: root.timelineInteractionActive
        controlsHovered: root.chromeControlsHovered
        controlsFocused: root.chromeControlsFocused
        popupOpen: root.popupOpen
        menuOpen: root.menuOpen
        drawerOpen: root.anyDrawerOpen
        modalActive: root.modalActive
        errorVisible: root.errorOverlayVisible
        fullScreen: root.fullScreen
    }

    PlayerCursorVisibilityController {
        id: cursorVisibilityController

        playing: root.playbackPlaying
        oscVisible: root.oscVisible
        scrubbing: root.timelineInteractionActive
        dragActive: root.controlsDragActive
        popupOpen: root.popupOpen
        menuOpen: root.menuOpen
        drawerOpen: root.anyDrawerOpen
        modalActive: root.modalActive
        errorVisible: root.errorOverlayVisible
        cursorHideSuppressed: root.cursorHideSuppressed
        windowActive: root.windowActive
        pointerInside: root.pointerInsideWindow
    }

    PlayerChromeActivityLayer {
        id: chromeActivityLayer

        anchors.fill: parent
        cursorHidden: root.cursorHidden

        onActivityDetected: function(reason) {
            chromeVisibilityController.notifyActivity(reason)
            cursorVisibilityController.notifyActivity(reason)
        }
    }

    PlayerTopRegion {
        id: topRegion
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: SpacingTokens.floatingTop
            leftMargin: SpacingTokens.windowSafeMinimum
            rightMargin: SpacingTokens.windowSafeMinimum
        }
        height: root.fullScreen
                ? LayoutTokens.headerHeightCompact
                : LayoutTokens.headerHeight

        PlayerFloatingHeader {
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            width: Math.min(LayoutTokens.headerWidth, parent.width)
            height: parent.height
            visible: !root.fullScreen && root.resolvedMediaTitle.trim().length > 0
            mediaTitle: root.resolvedMediaTitle
            metadataText: root.resolvedMediaMetadataText
            statusText: root.playbackStatusText
            windowExpanded: root.windowExpanded

            onMinimizeRequested: root.minimizeRequested()
            onMaximizeRestoreRequested: root.maximizeRestoreRequested()
            onCloseRequested: root.closeRequested()
        }

        FullscreenHeader {
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            width: Math.min(LayoutTokens.fullscreenHeaderWidth, parent.width)
            height: parent.height
            active: root.fullScreen
            mediaTitle: root.resolvedMediaTitle
        }
    }

    PlayerOverlayStack {
        id: overlayStack
        anchors.fill: parent

        PlayerStatusOverlay {
            anchors.fill: parent
            viewModel: root.statusViewModel
            suppressBuffering: root.timelineInteractionActive

            onOpenMediaRequested: root.openMediaRequested()
            onOpenUrlRequested: root.openUrlRequested()
            onCancelMediaOpenRequested: {
                if (root.transportViewModel !== null) {
                    root.transportViewModel.requestStop()
                }
            }
        }

        PlayerDropOverlay {
            id: dropOverlay
            anchors.fill: parent
            mediaDropHandler: root.mediaDropHandler
        }
    }

    PlayerHudOverlay {
        id: hudOverlay
        anchors.fill: parent
        viewModel: root.hudMessageQueue
        suppressed: root.errorOverlayVisible
    }

    PlayerBottomRegion {
        id: bottomRegion
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: SpacingTokens.floatingEdge
            rightMargin: SpacingTokens.floatingEdge
            bottomMargin: root.oscCompact
                          ? SpacingTokens.oscBottomCompact
                          : SpacingTokens.oscBottom
        }
        height: root.oscCompact
                ? LayoutTokens.oscHeightCompact
                : LayoutTokens.oscHeight
        opacity: root.oscVisible
                 ? OpacityTokens.visible
                 : OpacityTokens.hidden
        visible: root.oscVisible || opacity > OpacityTokens.hidden
        enabled: root.oscVisible

        PlayerOscLayout {
            id: playerOscLayout

            anchors.fill: parent
            compact: root.oscCompact
            inspectorOpen: root.playlistDrawerOpen && !root.fullScreen
            timelineContent: [
                TimelineControls {
                    anchors.fill: parent
                    compact: root.oscCompact
                    windowActive: root.windowActive
                    viewModel: root.timelineViewModel
                }
            ]
            transportContent: [
                TransportControls {
                    compact: root.oscCompact
                    viewModel: root.transportViewModel
                }
            ]
            volumeContent: [
                VolumeControls {
                    id: volumeControls

                    compact: root.oscCompact
                    viewModel: root.volumeViewModel
                }
            ]
            utilityContent: [
                PlayerUtilityControls {
                    compact: root.oscCompact
                    fullScreen: root.fullScreen
                    playlistOpen: root.playlistDrawerOpen

                    onTogglePlaylistRequested: root.playlistDrawerOpen = !root.playlistDrawerOpen
                    onToggleFullscreenRequested: root.fullscreenToggleRequested()
                }
            ]
        }

        Behavior on opacity {
            NumberAnimation {
                duration: root.oscVisible
                          ? MotionTokens.oscShowDuration
                          : MotionTokens.oscHideDuration
                easing.type: root.oscVisible
                             ? MotionTokens.enterEasingType
                             : MotionTokens.exitEasingType
                easing.bezierCurve: root.oscVisible
                                    ? MotionTokens.enterBezier
                                    : MotionTokens.exitBezier
            }
        }
    }

    PlayerDrawerHost {
        id: drawerHost
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
            topMargin: SpacingTokens.inspectorEdge
            rightMargin: SpacingTokens.inspectorRight
            bottomMargin: SpacingTokens.inspectorEdge
        }
        width: Math.min(
            LayoutTokens.inspectorWidth,
            Math.max(0, root.width - (SpacingTokens.windowSafeMinimum * 2)))

        PlaylistInspector {
            anchors.fill: parent
            opened: root.playlistDrawerOpen
            playlistModel: root.playlistModel
            playlistController: root.playlistController
            backdropSource: videoViewport
            backdropMappingRevision: drawerHost.x + drawerHost.y
                                     + drawerHost.width + drawerHost.height

            onCloseRequested: root.playlistDrawerOpen = false
            onAddMediaRequested: root.openMediaRequested()
        }
    }
}
