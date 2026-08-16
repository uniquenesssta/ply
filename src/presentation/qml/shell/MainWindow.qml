import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Player.Presentation.Theme

ApplicationWindow {
    id: window

    property var transportViewModel: null
    property var timelineViewModel: null
    property var volumeViewModel: null
    property var statusViewModel: null
    property var mediaViewModel: null
    property var hudMessageQueue: null
    property var playlistController: null
    property var playlistModel: null
    property var mediaOpenCoordinator: null
    property var urlOpenWorkflow: null
    property var mediaDropHandler: null

    width: LayoutTokens.windowDefaultWidth
    height: LayoutTokens.windowDefaultHeight
    minimumWidth: LayoutTokens.windowMinimumWidth
    minimumHeight: LayoutTokens.windowMinimumHeight
    visible: true
    title: qsTr("Player")
    color: ColorTokens.surfaceCanvas

    FullscreenWindowController {
        id: fullscreenWindowController
        targetWindow: window
    }

    LocalMediaOpenDialog {
        id: localMediaOpenDialog

        onLocalFileSelected: function(sourceUrl) {
            if (window.mediaOpenCoordinator !== null) {
                window.mediaOpenCoordinator.openLocalFile(sourceUrl)
            }
        }
    }

    UrlMediaOpenDialog {
        id: urlMediaOpenDialog

        onUrlSubmitted: function(sourceText) {
            if (window.urlOpenWorkflow === null) {
                urlMediaOpenDialog.errorKey = "workflow-unavailable"
                return
            }

            if (window.urlOpenWorkflow.openUrl(sourceText)) {
                urlMediaOpenDialog.close()
                return
            }

            urlMediaOpenDialog.errorKey = window.urlOpenWorkflow.lastErrorKey
        }
    }

    PlayerScreen {
        anchors.fill: parent
        transportViewModel: window.transportViewModel
        timelineViewModel: window.timelineViewModel
        volumeViewModel: window.volumeViewModel
        statusViewModel: window.statusViewModel
        mediaViewModel: window.mediaViewModel
        hudMessageQueue: window.hudMessageQueue
        mediaDropHandler: window.mediaDropHandler
        fullScreen: fullscreenWindowController.fullScreen
        windowActive: window.active
        modalActive: localMediaOpenDialog.visible || urlMediaOpenDialog.visible
        windowExpanded: window.visibility === Window.Maximized
                        || window.visibility === Window.FullScreen

        onOpenMediaRequested: localMediaOpenDialog.open()
        onOpenUrlRequested: urlMediaOpenDialog.open()
        onMinimizeRequested: window.showMinimized()
        onMaximizeRestoreRequested: {
            if (fullscreenWindowController.fullScreen) {
                fullscreenWindowController.exitFullscreen()
            } else if (window.visibility === Window.Maximized) {
                window.showNormal()
            } else {
                window.showMaximized()
            }
        }
        onFullscreenToggleRequested: fullscreenWindowController.toggleFullscreen()
        onCloseRequested: window.close()
    }
}
