import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    id: root

    property int count: 0

    signal addMediaRequested()

    objectName: "playlistFooter"

    Text {
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        text: qsTr("%1 items").arg(root.count)
        color: ColorTokens.textMuted
        font: TypographyTokens.metaBody
    }

    TextButton {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        text: qsTr("Add media")
        toolTipText: qsTr("Add media to the playlist")
        accessibleDescription: toolTipText

        onClicked: root.addMediaRequested()
    }
}
