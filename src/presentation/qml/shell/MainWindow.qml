import QtQuick
import QtQuick.Window
import Player.Presentation.Screens
import Player.Presentation.Theme

ApplicationWindow {
    id: root

    objectName: "mainWindow"
    visible: true
    title: qsTr("Qt6 libmpv Player")
    width: LayoutTokens.windowDefaultWidth
    height: LayoutTokens.windowDefaultHeight
    minimumWidth: LayoutTokens.windowMinimumWidth
    minimumHeight: LayoutTokens.windowMinimumHeight
    color: Theme.windowBackground

    PlayerScreen {
        anchors.fill: parent
        windowExpanded: root.visibility === Window.Maximized
                        || root.visibility === Window.FullScreen

        onMinimizeRequested: root.showMinimized()
        onMaximizeRestoreRequested: {
            if (root.visibility === Window.Maximized
                    || root.visibility === Window.FullScreen) {
                root.showNormal()
                return
            }
            root.showMaximized()
        }
        onCloseRequested: root.close()
    }
}
