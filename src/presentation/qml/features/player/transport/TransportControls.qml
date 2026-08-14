import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Row {
    id: root

    property var viewModel: null

    objectName: "playerTransportControls"
    spacing: SpacingTokens.controlTight

    IconButton {
        id: previousButton

        objectName: "transportPreviousButton"
        iconId: "previous"
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
        emphasis: IconButton.Primary
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
