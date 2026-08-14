import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property string mediaTitle: ""
    property string mediaMetadataText: ""
    property bool windowExpanded: false
    property var transportViewModel: null
    property var timelineViewModel: null
    property var volumeViewModel: null

    readonly property bool headerCompact: root.width < LayoutTokens.windowMinimumWidth
    readonly property real oscAvailableWidth: Math.max(
        0,
        root.width - (SpacingTokens.floatingEdge * 2))
    readonly property bool oscCompact: root.oscAvailableWidth < LayoutTokens.oscMaximumWidth

    signal minimizeRequested()
    signal maximizeRestoreRequested()
    signal closeRequested()

    objectName: "playerScreen"

    VideoViewport {
        id: videoViewport
        anchors.fill: parent
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
        height: root.headerCompact
                ? LayoutTokens.headerHeightCompact
                : LayoutTokens.headerHeight

        PlayerFloatingHeader {
            anchors.fill: parent
            mediaTitle: root.mediaTitle
            metadataText: root.mediaMetadataText
            windowExpanded: root.windowExpanded

            onMinimizeRequested: root.minimizeRequested()
            onMaximizeRestoreRequested: root.maximizeRestoreRequested()
            onCloseRequested: root.closeRequested()
        }
    }

    PlayerOverlayStack {
        id: overlayStack
        anchors.fill: parent
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

        PlayerOscLayout {
            anchors.fill: parent
            compact: root.oscCompact
            timelineContent: [
                TimelineControls {
                    anchors.fill: parent
                    compact: root.oscCompact
                    viewModel: root.timelineViewModel
                }
            ]
            transportContent: [
                TransportControls {
                    viewModel: root.transportViewModel
                }
            ]
            volumeContent: [
                VolumeControls {
                    compact: root.oscCompact
                    viewModel: root.volumeViewModel
                }
            ]
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
    }
}
