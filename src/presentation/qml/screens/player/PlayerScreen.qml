import QtQuick
import Player.Presentation.Theme

Item {
    id: root

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
            leftMargin: SpacingTokens.floatingEdge
            rightMargin: SpacingTokens.floatingEdge
        }
        height: LayoutTokens.headerHeight
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
            bottomMargin: SpacingTokens.oscBottom
        }
        height: LayoutTokens.oscHeight
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
