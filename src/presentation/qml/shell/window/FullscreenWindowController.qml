import QtQuick
import QtQuick.Window

Item {
    id: root

    property var targetWindow: null
    property int restoreVisibility: Window.Windowed

    readonly property bool fullScreen: root.targetWindow !== null
                                       && root.targetWindow.visibility === Window.FullScreen

    objectName: "fullscreenWindowController"
    width: 0
    height: 0

    function enterFullscreen() {
        if (root.targetWindow === null || root.fullScreen) {
            return
        }

        root.restoreVisibility = root.targetWindow.visibility === Window.Maximized
                               ? Window.Maximized
                               : Window.Windowed
        root.targetWindow.showFullScreen()
    }

    function exitFullscreen() {
        if (root.targetWindow === null || !root.fullScreen) {
            return
        }

        if (root.restoreVisibility === Window.Maximized) {
            root.targetWindow.showMaximized()
        } else {
            root.targetWindow.showNormal()
        }
    }

    function toggleFullscreen() {
        if (root.fullScreen) {
            root.exitFullscreen()
        } else {
            root.enterFullscreen()
        }
    }

    Shortcut {
        sequence: "Esc"
        enabled: root.fullScreen
        context: Qt.ApplicationShortcut

        onActivated: root.exitFullscreen()
    }
}
