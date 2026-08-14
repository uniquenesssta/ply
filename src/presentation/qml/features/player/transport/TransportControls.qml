import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Row {
    id: root

    property var viewModel: null
    property bool compact: false

    objectName: "playerTransportControls"
    spacing: root.compact
             ? SpacingTokens.fullscreenTransportGap
             : SpacingTokens.controlTight

    IconButton {
        id: previousButton

        objectName: "transportPreviousButton"
        iconId: "previous"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        enabled: root.viewModel !== null && root.viewModel.canPrevious
        toolTipText: qsTr("Previous")
        accessibleName: toolTipText

        onClicked: {
            if (root.viewModel !== null) {
                root.viewModel.requestPrevious()
            }
        }
    }

    IconButton {
        id: playPauseButton

        objectName: "transportPlayPauseButton"
        iconId: "play"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        emphasis: root.compact
                  ? IconButton.Secondary
                  : IconButton.Primary
        enabled: root.viewModel !== null
                 && (root.viewModel.canPlay || root.viewModel.canPause)
        toolTipText: root.viewModel !== null && root.viewModel.isPlaying
                     ? qsTr("Pause")
                     : qsTr("Play")
        accessibleName: toolTipText

        onClicked: {
            if (root.viewModel !== null) {
                root.viewModel.requestTogglePlayPause()
            }
        }
    }

    IconButton {
        id: nextButton

        objectName: "transportNextButton"
        iconId: "next"
        iconSizeOverride: root.compact ? LayoutTokens.controlIconCompact : 0
        enabled: root.viewModel !== null && root.viewModel.canNext
        toolTipText: qsTr("Next")
        accessibleName: toolTipText

        onClicked: {
            if (root.viewModel !== null) {
                root.viewModel.requestNext()
            }
        }
    }
}
