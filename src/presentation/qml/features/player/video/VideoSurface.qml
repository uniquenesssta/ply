import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    clip: true

    Rectangle {
        anchors.fill: parent
        color: Theme.videoBackground
    }

    MpvVideoItem {
        id: videoItem
        objectName: "mpvVideoItem"
        anchors.fill: parent
    }
}
