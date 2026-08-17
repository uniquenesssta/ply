import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Row {
    id: root

    property bool fullScreen: false

    signal toggleFullscreenRequested()

    objectName: "playerFullscreenControls"
    width: implicitWidth
    height: implicitHeight

    IconButton {
        id: fullscreenButton

        objectName: "fullscreenToggleButton"
        iconId: "fullscreen"
        iconSizeOverride: root.fullScreen ? LayoutTokens.controlIconCompact : 0
        toolTipText: root.fullScreen
                     ? qsTr("Exit Fullscreen")
                     : qsTr("Enter Fullscreen")
        accessibleName: toolTipText
        accessibleDescription: root.fullScreen
                               ? qsTr("Return to the previous window mode")
                               : qsTr("Fill the current screen with the player")

        onClicked: root.toggleFullscreenRequested()
    }
}
