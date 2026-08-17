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

    PlaylistControls {
        opened: root.playlistOpen
        compact: root.compact

        onToggleRequested: root.togglePlaylistRequested()
    }

    FullscreenControls {
        fullScreen: root.fullScreen

        onToggleFullscreenRequested: root.toggleFullscreenRequested()
    }
}
