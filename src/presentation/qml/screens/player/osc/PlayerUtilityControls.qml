import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

Row {
    id: root

    property bool compact: false
    property bool fullScreen: false
    property bool playlistOpen: false
    property bool mediaAvailable: false
    property bool externalSubtitleAvailable: false
    property var audioTrackModel: null
    property var subtitleTrackModel: null
    property var trackSelectionController: null
    property var subtitleDelayController: null
    readonly property bool trackPopupOpen: trackPopup.opened

    signal togglePlaylistRequested()
    signal toggleFullscreenRequested()
    signal openExternalSubtitleRequested()

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
        accessibleDescription: qsTr("Choose audio/subtitle tracks, turn subtitles off, add an external subtitle, or adjust subtitle delay")
        enabled: root.mediaAvailable
                 && (root.externalSubtitleAvailable
                     || (root.trackSelectionController !== null
                         && ((root.audioTrackModel !== null && root.audioTrackModel.count > 0)
                             || (root.subtitleTrackModel !== null && root.subtitleTrackModel.count > 0))))

        onClicked: trackPopup.opened ? trackPopup.close() : trackPopup.open()
    }

    TrackSelectionPopup {
        id: trackPopup

        parent: root
        x: 0
        y: -height - SpacingTokens.controlTight
        audioTrackModel: root.audioTrackModel
        subtitleTrackModel: root.subtitleTrackModel
        trackSelectionController: root.trackSelectionController
        subtitleDelayController: root.subtitleDelayController
        externalSubtitleAvailable: root.externalSubtitleAvailable

        onAddExternalSubtitleRequested: root.openExternalSubtitleRequested()
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
