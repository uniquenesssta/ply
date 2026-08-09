import QtQuick

Item {
    id: root

    clip: true

    MpvVideoItem {
        id: videoItem
        objectName: "mpvVideoItem"
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        z: 1
        color: Theme.videoBackground

        Text {
            anchors.centerIn: parent
            text: qsTr("Video surface ready")
            color: Theme.secondaryText
            font.pixelSize: 15
        }
    }
}
