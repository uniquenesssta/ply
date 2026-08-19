import QtQuick
import QtQuick.Controls
import Player.Presentation.Theme

Popup {
    id: root

    property var audioTrackModel: null
    property var subtitleTrackModel: null
    property var trackSelectionController: null

    signal selectionSubmissionRejected()

    width: LayoutTokens.trackSelectionPopupWidth
    padding: SpacingTokens.trackSelectionPopupPadding
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    function trackLabel(title, language, codec, trackId, fallback) {
        const primary = title.trim().length > 0
                        ? title
                        : language.trim().length > 0
                          ? language
                          : fallback + " " + trackId
        return codec.trim().length > 0 ? primary + " · " + codec : primary
    }

    contentItem: Column {
        spacing: SpacingTokens.trackSelectionPopupGap

        Label {
            text: qsTr("Audio")
        }

        Repeater {
            model: root.audioTrackModel

            delegate: Button {
                required property var trackId
                required property string title
                required property string language
                required property string codec
                required property bool selected

                width: root.width - root.leftPadding - root.rightPadding
                text: (selected ? "✓ " : "")
                      + root.trackLabel(title, language, codec, trackId, qsTr("Audio Track"))

                onClicked: {
                    if (root.trackSelectionController !== null
                            && root.trackSelectionController.selectAudioTrack(trackId)) {
                        root.close()
                    } else {
                        root.selectionSubmissionRejected()
                    }
                }
            }
        }

        Label {
            text: qsTr("Subtitles")
        }

        Button {
            width: root.width - root.leftPadding - root.rightPadding
            text: ((root.subtitleTrackModel === null
                    || root.subtitleTrackModel.selectedTrackId === 0) ? "✓ " : "")
                  + qsTr("Off")

            onClicked: {
                if (root.trackSelectionController !== null
                        && root.trackSelectionController.disableSubtitles()) {
                    root.close()
                } else {
                    root.selectionSubmissionRejected()
                }
            }
        }

        Repeater {
            model: root.subtitleTrackModel

            delegate: Button {
                required property var trackId
                required property string title
                required property string language
                required property string codec
                required property bool selected

                width: root.width - root.leftPadding - root.rightPadding
                text: (selected ? "✓ " : "")
                      + root.trackLabel(title, language, codec, trackId, qsTr("Subtitle Track"))

                onClicked: {
                    if (root.trackSelectionController !== null
                            && root.trackSelectionController.selectSubtitleTrack(trackId)) {
                        root.close()
                    } else {
                        root.selectionSubmissionRejected()
                    }
                }
            }
        }
    }
}
