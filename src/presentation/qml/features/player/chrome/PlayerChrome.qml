import QtQuick
import Player.Presentation.Theme

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

        Text {
            anchors {
                left: parent.left
                leftMargin: SpacingTokens.headerContent
                verticalCenter: parent.verticalCenter
            }
            text: qsTr("Player framework")
            color: ColorTokens.textPrimary
            font: TypographyTokens.mediaTitle
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

        Text {
            anchors.centerIn: parent
            text: qsTr("Playback controls will be added after the player state boundary is implemented")
            color: ColorTokens.textSecondary
            font: TypographyTokens.controlBody
        }
    }
}
