import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives

Item {
    id: root

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: LayoutTokens.headerHeight
        color: ColorTokens.surfaceGlass

        TitleText {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: SpacingTokens.headerContent
                rightMargin: SpacingTokens.headerActionsInset
                verticalCenter: parent.verticalCenter
            }
            text: qsTr("Player framework")
            variant: TitleText.Media
        }
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: LayoutTokens.oscHeight
        color: ColorTokens.surfaceGlass

        BodyText {
            anchors.centerIn: parent
            width: parent.width - (SpacingTokens.oscInset * 2)
            text: qsTr("Playback controls will be added after the player state boundary is implemented")
            variant: BodyText.Control
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
