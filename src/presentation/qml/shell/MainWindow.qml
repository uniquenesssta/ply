import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window

    width: 1280
    height: 720
    minimumWidth: 960
    minimumHeight: 540
    visible: true
    title: qsTr("Player")
    color: Theme.windowBackground

    PlayerScreen {
        anchors.fill: parent
    }
}
