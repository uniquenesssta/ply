import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property string mediaTitle: ""
    property string metadataText: ""
    property bool windowExpanded: false

    readonly property bool compact: root.height <= LayoutTokens.headerHeightCompact
    readonly property int preferredInfoWidth: root.compact
                                               ? LayoutTokens.headerInfoWidthCompact
                                               : LayoutTokens.headerInfoWidth

    signal minimizeRequested()
    signal maximizeRestoreRequested()
    signal closeRequested()

    objectName: "playerFloatingHeader"
    implicitHeight: LayoutTokens.headerHeight

    WindowActionsPod {
        id: windowActions

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        compact: root.compact
        windowExpanded: root.windowExpanded

        onMinimizeRequested: root.minimizeRequested()
        onMaximizeRestoreRequested: root.maximizeRestoreRequested()
        onCloseRequested: root.closeRequested()
    }

    MediaInfoPod {
        id: mediaInfo

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        width: Math.min(
            root.preferredInfoWidth,
            Math.max(
                0,
                root.width
                    - windowActions.width
                    - SpacingTokens.controlAdjacent))
        compact: root.compact
        mediaTitle: root.mediaTitle
        metadataText: root.metadataText
    }
}
