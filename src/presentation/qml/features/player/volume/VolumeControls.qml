import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Row {
    id: root

    property var viewModel: null
    property bool compact: false

    objectName: "playerVolumeControls"

    function syncSliderFromViewModel() {
        if (volumeSlider.pressed) {
            return
        }

        const normalized = root.viewModel !== null
                         ? root.viewModel.normalizedVolume
                         : 0.0
        if (Math.abs(volumeSlider.value - normalized) > 0.0000001) {
            volumeSlider.value = normalized
        }
    }

    IconButton {
        id: muteButton

        objectName: "volumeMuteButton"
        iconId: "volume"
        enabled: root.viewModel !== null
                 && root.viewModel.canToggleMute
                 && !root.viewModel.mutePending
        toolTipText: root.viewModel !== null && root.viewModel.muted
                     ? qsTr("Unmute")
                     : qsTr("Mute")
        accessibleName: toolTipText
        accessibleDescription: root.viewModel !== null && root.viewModel.muted
                               ? qsTr("Audio is muted")
                               : qsTr("Audio is unmuted")

        onClicked: {
            if (root.viewModel !== null) {
                root.viewModel.requestToggleMuted()
            }
        }
    }

    Slider {
        id: volumeSlider

        objectName: "volumeSlider"
        visible: !root.compact
        width: LayoutTokens.volumeTrackWidth
               + (LayoutTokens.sliderTrackInset * 2)
        height: LayoutTokens.controlHitMinimum
        value: 0.0
        stepSize: 0.01
        wheelStep: stepSize
        showValue: false
        wheelEnabled: true
        enabled: root.viewModel !== null && root.viewModel.canAdjustVolume
        accessibleName: qsTr("Volume")
        accessibleDescription: qsTr("Adjust playback volume")

        onValueEdited: function(nextValue) {
            if (root.viewModel !== null) {
                root.viewModel.requestVolumeNormalized(nextValue)
            }
        }
    }

    Connections {
        target: root.viewModel

        function onStateChanged() {
            root.syncSliderFromViewModel()
        }
    }

    onViewModelChanged: root.syncSliderFromViewModel()
    Component.onCompleted: root.syncSliderFromViewModel()
}
