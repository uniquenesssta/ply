import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Theme

Row {
    id: root

    property bool compact: false
    property bool fullScreen: false
    property bool playlistOpen: false

    signal togglePlaylistRequested()
    signal toggleFullscreenRequested()

    objectName: "playerUtilityControls"
    spacing: root.compact
             ? SpacingTokens.fullscreenTransportGap
             : SpacingTokens.utilityControlGap
    width: implicitWidth
    height: implicitHeight

    Item {
        id: subtitlesPlaceholder

        objectName: "subtitlesUnavailableControl"
        width: LayoutTokens.controlHitMinimum
        height: LayoutTokens.controlHitMinimum

        Icon {
            anchors.centerIn: parent
            width: root.compact
                   ? LayoutTokens.controlIconCompact
                   : LayoutTokens.controlIcon
            height: width
            iconId: "subtitles"
            color: ColorTokens.iconPrimary
        }
    }

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
