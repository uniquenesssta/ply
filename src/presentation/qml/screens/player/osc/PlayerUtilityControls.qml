import QtQuick
import QtQuick.Controls.Basic
import Player.Presentation.Primitives
import Player.Presentation.Theme
import Player.Presentation.Controls

Row {
    id: root

    property bool compact: false
    property bool fullScreen: false
    property bool playlistOpen: false

    property var subtitleTrackListModel: null
    property var audioTrackListModel: null
    property var chapterListModel: null
    property var trackSelectionController: null
    property var subtitleDelayController: null
    property var audioDelayController: null
    property var externalSubtitleLoader: null
    property var timelineViewModel: null

    signal togglePlaylistRequested()
    signal toggleFullscreenRequested()

    objectName: "playerUtilityControls"
    spacing: root.compact
             ? SpacingTokens.fullscreenTransportGap
             : SpacingTokens.utilityControlGap
    width: implicitWidth
    height: implicitHeight

    IconButton {
        id: audioButton

        objectName: "audioTracksControl"
        iconId: "audio"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        toolTipText: qsTr("Audio tracks")
        accessibleName: toolTipText
        accessibleDescription: qsTr("Choose audio track and audio delay")
        enabled: root.trackSelectionController !== null
                 && root.trackSelectionController.canSelectAudio

        onClicked: audioMenu.open()
    }

    IconButton {
        id: subtitlesButton

        objectName: "subtitlesControl"
        iconId: "subtitles"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        toolTipText: qsTr("Subtitles")
        accessibleName: toolTipText
        accessibleDescription: qsTr("Choose subtitle track, load a subtitle file and adjust subtitle delay")
        enabled: root.trackSelectionController !== null
                 && (root.trackSelectionController.canSelectSubtitle
                     || (root.externalSubtitleLoader !== null
                         && root.externalSubtitleLoader.canLoad))

        onClicked: subtitleMenu.open()
    }

    IconButton {
        id: chaptersButton

        objectName: "chaptersControl"
        iconId: "chapters"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        toolTipText: qsTr("Chapters")
        accessibleName: toolTipText
        accessibleDescription: qsTr("Jump to a chapter")
        enabled: root.chapterListModel !== null && root.chapterListModel.count > 0

        onClicked: chapterMenu.open()
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

    SubtitleMenu {
        id: subtitleMenu

        parent: root.Window.contentItem
        x: Math.max(0, Math.min(
            root.mapToItem(root.Window.contentItem, 0, 0).x,
            root.Window.width - width))
        y: Math.max(0, root.mapToItem(root.Window.contentItem, 0, 0).y - height)
        trackListModel: root.subtitleTrackListModel
        selectionController: root.trackSelectionController
        delayController: root.subtitleDelayController
        subtitleLoader: root.externalSubtitleLoader
    }

    AudioMenu {
        id: audioMenu

        parent: root.Window.contentItem
        x: Math.max(0, Math.min(
            root.mapToItem(root.Window.contentItem, 0, 0).x,
            root.Window.width - width))
        y: Math.max(0, root.mapToItem(root.Window.contentItem, 0, 0).y - height)
        trackListModel: root.audioTrackListModel
        selectionController: root.trackSelectionController
        delayController: root.audioDelayController
    }

    ChapterMenu {
        id: chapterMenu

        parent: root.Window.contentItem
        x: Math.max(0, Math.min(
            root.mapToItem(root.Window.contentItem, 0, 0).x,
            root.Window.width - width))
        y: Math.max(0, root.mapToItem(root.Window.contentItem, 0, 0).y - height)
        chapterListModel: root.chapterListModel
        timelineViewModel: root.timelineViewModel
    }
}
