import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Player.Presentation.Theme

ApplicationWindow {
    id: window

    property var transportViewModel: null
    property var timelineViewModel: null
    property var volumeViewModel: null

    width: LayoutTokens.windowDefaultWidth
    height: LayoutTokens.windowDefaultHeight
    minimumWidth: LayoutTokens.windowMinimumWidth
    minimumHeight: LayoutTokens.windowMinimumHeight
    visible: true
    title: qsTr("Player")
    color: ColorTokens.surfaceCanvas

    PlayerScreen {
        anchors.fill: parent
        transportViewModel: window.transportViewModel
        timelineViewModel: window.timelineViewModel
        volumeViewModel: window.volumeViewModel
        windowExpanded: window.visibility === Window.Maximized
                        || window.visibility === Window.FullScreen

        onMinimizeRequested: window.showMinimized()
        onMaximizeRestoreRequested: {
            if (window.visibility === Window.Maximized
                    || window.visibility === Window.FullScreen) {
                window.showNormal()
            } else {
                window.showMaximized()
            }
        }
        onCloseRequested: window.close()
    }
}
