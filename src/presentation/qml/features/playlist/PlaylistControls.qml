import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Row {
    id: root

    property bool opened: false
    property bool compact: false

    signal toggleRequested()

    objectName: "playlistControls"

    IconButton {
        objectName: "playlistToggleButton"
        iconId: "playlist"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        toolTipText: root.opened ? qsTr("Hide Playlist") : qsTr("Show Playlist")
        accessibleName: toolTipText
        accessibleDescription: qsTr("Toggle the playlist inspector")

        onClicked: root.toggleRequested()
    }
}
