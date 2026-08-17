import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property string mediaTitle: ""
    property string metadataText: ""
    property string statusText: ""
    property bool windowExpanded: false

    readonly property bool compact: root.height <= LayoutTokens.headerHeightCompact
    readonly property int preferredInfoWidth: root.compact
                                               ? LayoutTokens.headerInfoWidthCompact
                                               : LayoutTokens.headerInfoWidth
    readonly property int podHeight: root.compact
                                     ? LayoutTokens.headerHeightCompact
                                     : LayoutTokens.headerHeight

    signal minimizeRequested()
    signal maximizeRestoreRequested()
    signal closeRequested()

    objectName: "playerFloatingHeader"
    visible: root.mediaTitle.trim().length > 0
    implicitWidth: LayoutTokens.headerWidth
    implicitHeight: LayoutTokens.headerHeight
    cornerRadius: RadiusTokens.header
    fillAlpha: MaterialTokens.headerFillAlpha
    backdropBlurRadius: MaterialTokens.headerBlur
    contentPadding: SpacingTokens.none

    TitleText {
        anchors {
            left: parent.left
            right: statusLabel.left
            top: parent.top
            leftMargin: SpacingTokens.headerContent
            rightMargin: SpacingTokens.controlAdjacent
            topMargin: SpacingTokens.headerTitleTop
        }
        text: root.mediaTitle
        variant: TitleText.Media
    }

    CaptionText {
        anchors {
            left: parent.left
            right: statusLabel.left
            top: parent.top
            leftMargin: SpacingTokens.headerContent
            rightMargin: SpacingTokens.controlAdjacent
            topMargin: SpacingTokens.headerMetaTop
        }
        visible: root.metadataText.trim().length > 0
        text: root.metadataText
        variant: CaptionText.Technical
        color: ColorTokens.textSecondary
    }

    CaptionText {
        id: statusLabel

        x: Math.min(LayoutTokens.headerStatusX,
                    Math.max(SpacingTokens.none,
                             closeButton.x - implicitWidth - SpacingTokens.controlAdjacent))
        anchors.top: parent.top
        anchors.topMargin: SpacingTokens.headerStatusTop
        visible: root.statusText.trim().length > 0
        text: root.statusText
        variant: CaptionText.MicroStrong
        color: ColorTokens.accentStrong
    }

    IconButton {
        id: closeButton

        objectName: "playerWindowCloseButton"
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: SpacingTokens.headerCloseRight
        }
        iconId: "close"
        toolTipText: qsTr("Close")
        accessibleDescription: qsTr("Close the player window")

        onClicked: root.closeRequested()
    }
}
