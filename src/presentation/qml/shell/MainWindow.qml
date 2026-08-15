import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Player.Presentation.Theme

ApplicationWindow {
    id: window

    property var transportViewModel: null
    property var timelineViewModel: null
    property var volumeViewModel: null
    property var statusViewModel: null
    property var hudMessageQueue: null

    width: LayoutTokens.windowDefaultWidth
    height: LayoutTokens.windowDefaultHeight
    minimumWidth: LayoutTokens.windowMinimumWidth
    minimumHeight: LayoutTokens.windowMinimumHeight
    visible: true
    title: qsTr("Player")
    color: ColorTokens.surfaceCanvas

    FullscreenWindowController {
        id: fullscreenWindowController
        targetWindow: window
    }

    PlayerScreen {
        anchors.fill: parent
        transportViewModel: window.transportViewModel
        timelineViewModel: window.timelineViewModel
        volumeViewModel: window.volumeViewModel
        statusViewModel: window.statusViewModel
        hudMessageQueue: window.hudMessageQueue
        fullScreen: fullscreenWindowController.fullScreen
        windowExpanded: window.visibility === Window.Maximized
                        || window.visibility === Window.FullScreen

        onMinimizeRequested: window.showMinimized()
        onMaximizeRestoreRequested: {
            if (fullscreenWindowController.fullScreen) {
                fullscreenWindowController.exitFullscreen()
            } else if (window.visibility === Window.Maximized) {
                window.showNormal()
            } else {
                window.showMaximized()
            }
        }
        onFullscreenToggleRequested: fullscreenWindowController.toggleFullscreen()
        onCloseRequested: window.close()
    }
}
