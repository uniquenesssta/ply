import QtQuick
import QtQuick.Controls.Basic
import Player.Presentation.Theme
import Player.Presentation.Controls

// Audio track menu: select from the read-only snapshot model and nudge/reset
// the audio delay. Intents flow through C++ controllers only.
Popup {
    id: root

    property var trackListModel: null
    property var selectionController: null
    property var delayController: null

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
            text: qsTr("Audio tracks")
            horizontalAlignment: Text.AlignLeft
        }

        ListView {
            id: trackList

            objectName: "audioTrackList"
            width: parent.width
            height: Math.min(
                contentHeight,
                LayoutTokens.windowMinimumHeight * 0.28)
            clip: true
            interactive: contentHeight > height
            model: root.trackListModel

            delegate: TextButton {
                objectName: "audioTrackRow"
                width: trackList.width
                text: model.title.length > 0
                      ? model.title
                      : qsTr("Track %1").arg(model.trackId)
                textColor: model.selected
                           ? ColorTokens.textEmphasis
                           : ColorTokens.textPrimary
                enabled: root.selectionController !== null
                         && root.selectionController.canSelectAudio

                onClicked: {
                    if (root.selectionController !== null) {
                        root.selectionController.requestSelectAudioTrack(model.trackId)
                        root.close()
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: SpacingTokens.controlAdjacent

            TextButton {
                objectName: "audioDelayEarlierButton"
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
                objectName: "audioDelayLaterButton"
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

                objectName: "audioDelayResetButton"
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
}
