import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import Player.Presentation.Theme
import Player.Presentation.Controls

// Subtitle track menu: select/off from the read-only snapshot model, load an
// external subtitle file, and nudge/reset the subtitle delay. All intents
// flow through C++ controllers; no QML-to-mpv path exists.
Popup {
    id: root

    property var trackListModel: null
    property var selectionController: null
    property var delayController: null
    property var subtitleLoader: null

    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: SpacingTokens.surfacePaddingSmall

    width: Math.min(
        320,
        parent !== null ? parent.width : 320)
    implicitHeight: contentColumn.implicitHeight + (SpacingTokens.surfacePaddingSmall * 2)

    background: Rectangle {
        radius: RadiusTokens.surfacePopover
        color: Qt.rgba(ColorTokens.surfaceGlassStrong.r,
                       ColorTokens.surfaceGlassStrong.g,
                       ColorTokens.surfaceGlassStrong.b,
                       MaterialTokens.popoverFillAlpha)
        border.width: LayoutTokens.surfaceBorderWidth
        border.color: Qt.rgba(ColorTokens.borderGlass.r,
                              ColorTokens.borderGlass.g,
                              ColorTokens.borderGlass.b,
                              MaterialTokens.borderStrongAlpha)
    }

    contentItem: Column {
        id: contentColumn

        spacing: SpacingTokens.controlAdjacent

        CaptionText {
            width: parent.width
            text: qsTr("Subtitles")
            horizontalAlignment: Text.AlignLeft
        }

        ListView {
            id: trackList

            objectName: "subtitleTrackList"
            width: parent.width
            height: Math.min(
                contentHeight,
                LayoutTokens.windowMinimumHeight * 0.28)
            clip: true
            interactive: contentHeight > height
            model: root.trackListModel

            delegate: TextButton {
                objectName: "subtitleTrackRow"
                width: trackList.width
                text: model.title.length > 0
                      ? model.title
                      : qsTr("Track %1").arg(model.trackId)
                textColor: model.selected
                           ? ColorTokens.textEmphasis
                           : ColorTokens.textPrimary
                enabled: root.selectionController !== null
                         && root.selectionController.canSelectSubtitle

                onClicked: {
                    if (root.selectionController !== null) {
                        root.selectionController.requestSelectSubtitleTrack(model.trackId)
                        root.close()
                    }
                }
            }
        }

        TextButton {
            objectName: "subtitleOffButton"
            width: parent.width
            text: qsTr("Off")
            enabled: root.selectionController !== null
                     && root.selectionController.canSelectSubtitle

            onClicked: {
                if (root.selectionController !== null) {
                    root.selectionController.requestSelectSubtitleTrack(0)
                    root.close()
                }
            }
        }

        TextButton {
            objectName: "loadExternalSubtitleButton"
            width: parent.width
            text: qsTr("Load subtitle file…")
            enabled: root.subtitleLoader !== null && root.subtitleLoader.canLoad

            onClicked: subtitleFileDialog.open()
        }

        Row {
            width: parent.width
            spacing: SpacingTokens.controlAdjacent

            TextButton {
                objectName: "subtitleDelayEarlierButton"
                width: (parent.width - parent.spacing) / 2
                text: qsTr("Delay −")
                enabled: root.delayController !== null && root.delayController.canAdjust
                onClicked: {
                    if (root.delayController !== null) {
                        root.delayController.nudgeEarlier()
                    }
                }
            }

            TextButton {
                objectName: "subtitleDelayLaterButton"
                width: (parent.width - parent.spacing) / 2
                text: qsTr("Delay +")
                enabled: root.delayController !== null && root.delayController.canAdjust
                onClicked: {
                    if (root.delayController !== null) {
                        root.delayController.nudgeLater()
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: SpacingTokens.controlAdjacent

            CaptionText {
                width: parent.width - (parent.spacing + resetButton.width)
                text: root.delayController !== null
                      ? qsTr("Delay: %1").arg(root.delayController.delayText)
                      : ""
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }

            TextButton {
                id: resetButton

                objectName: "subtitleDelayResetButton"
                text: qsTr("Reset")
                enabled: root.delayController !== null && root.delayController.canAdjust
                onClicked: {
                    if (root.delayController !== null) {
                        root.delayController.reset()
                    }
                }
            }
        }
    }

    FileDialog {
        id: subtitleFileDialog

        title: qsTr("Load subtitle file")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Subtitle files (*.srt *.ass *.ssa *.sub *.vtt)")]

        onAccepted: {
            if (root.subtitleLoader !== null) {
                root.subtitleLoader.requestLoad(subtitleFileDialog.selectedFile)
            }
            root.close()
        }
    }
}
