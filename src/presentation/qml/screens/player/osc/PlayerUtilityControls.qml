import QtQuick
import Player.Presentation.Theme

Row {
    id: root

    property bool compact: false
    property bool fullScreen: false
    property bool playlistOpen: false

    signal togglePlaylistRequested()
    signal toggleFullscreenRequested()

    objectName: "playerUtilityControls"
    spacing: SpacingTokens.controlTight
    width: implicitWidth
    height: implicitHeight

    PlaylistControls {
        id: playlistControls

        opened: root.playlistOpen
        compact: root.compact
        width: implicitWidth
        height: implicitHeight

        onToggleRequested: root.togglePlaylistRequested()
    }

    FullscreenControls {
        id: fullscreenControls

        fullScreen: root.fullScreen
        width: implicitWidth
        height: implicitHeight

        onToggleFullscreenRequested: root.toggleFullscreenRequested()
    }
}
