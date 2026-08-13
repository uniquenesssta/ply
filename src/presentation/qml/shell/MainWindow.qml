import QtQuick
import QtQuick.Controls
import Player.Presentation.Theme

ApplicationWindow {
    id: window

    width: LayoutTokens.windowDefaultWidth
    height: LayoutTokens.windowDefaultHeight
    minimumWidth: LayoutTokens.windowMinimumWidth
    minimumHeight: LayoutTokens.windowMinimumHeight
    visible: true
    title: qsTr("Player")
    color: ColorTokens.surfaceCanvas

    PlayerScreen {
        anchors.fill: parent
    }
}
