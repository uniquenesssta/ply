import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

Row {
    id: root

    property bool compact: false
    property bool fullScreen: false
    property bool playlistOpen: false
    property var audioTrackModel: null
    property var subtitleTrackModel: null
    property var trackSelectionController: null
    readonly property bool trackPopupOpen: trackPopup.opened

    signal togglePlaylistRequested()
    signal toggleFullscreenRequested()

    objectName: "playerUtilityControls"
    spacing: root.compact
             ? SpacingTokens.fullscreenTransportGap
             : SpacingTokens.utilityControlGap
    width: implicitWidth
    height: implicitHeight

    IconButton {
        id: trackButton

        objectName: "trackSelectionButton"
        iconId: "subtitles"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        toolTipText: qsTr("Audio and Subtitles")
        accessibleName: toolTipText
        accessibleDescription: qsTr("Choose an audio track, subtitle track, or turn subtitles off")
        enabled: root.trackSelectionController !== null
                 && ((root.audioTrackModel !== null && root.audioTrackModel.count > 0)
                     || (root.subtitleTrackModel !== null && root.subtitleTrackModel.count > 0))

        onClicked: trackPopup.opened ? trackPopup.close() : trackPopup.open()
    }

    TrackSelectionPopup {
        id: trackPopup

        parent: root
        x: 0
        y: -height - SpacingTokens.controlGap
        audioTrackModel: root.audioTrackModel
        subtitleTrackModel: root.subtitleTrackModel
        trackSelectionController: root.trackSelectionController
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
