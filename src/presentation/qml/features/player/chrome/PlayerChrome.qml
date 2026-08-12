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
        height: 56
        color: Theme.chromeBackground

        Text {
            anchors {
                left: parent.left
                leftMargin: 20
                verticalCenter: parent.verticalCenter
            }
            text: qsTr("Player framework")
            color: Theme.primaryText
            font.pixelSize: 15
            font.weight: Font.DemiBold
        }
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 72
        color: Theme.chromeBackground

        Text {
            anchors.centerIn: parent
            text: qsTr("Playback controls will be added after the player state boundary is implemented")
            color: Theme.secondaryText
            font.pixelSize: 13
        }
    }
}
