import QtQuick

Rectangle {
    id: root

    color: Theme.videoBackground

    Text {
        anchors.centerIn: parent
        text: qsTr("libmpv Render API will be connected in a later Atomic Task")
        color: Theme.secondaryText
        font.pixelSize: 15
    }
}
