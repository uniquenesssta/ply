import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    id: root

    property var viewModel: null
    property bool compact: false

    readonly property bool timelineEnabled: root.viewModel !== null
                                            && root.viewModel.canSeek
    readonly property font currentTimeFont: root.compact
                                             ? TypographyTokens.timecodeSmallPrimary
                                             : TypographyTokens.timecodeMediumPrimary
    readonly property font durationTimeFont: root.compact
                                              ? TypographyTokens.timecodeSmallSecondary
                                              : TypographyTokens.timecodeMediumSecondary

    objectName: "timelineControls"
    implicitHeight: root.compact
                    ? LayoutTokens.oscTimelineLaneHeightCompact
                    : LayoutTokens.oscTimelineLaneHeight

    function syncSliderFromViewModel() {
        if (timelineSlider.pressed) {
            return
        }

        const normalized = root.viewModel !== null
                         ? root.viewModel.displayedNormalized
                         : 0.0
        if (Math.abs(timelineSlider.value - normalized) > 0.0000001) {
            timelineSlider.value = normalized
        }
    }

    Slider {
        id: timelineSlider

        objectName: "timelineSlider"
        x: -LayoutTokens.sliderTrackInset
        y: root.compact
           ? ((root.height - height) / 2)
             + (LayoutTokens.sliderTrackHeight / 2)
           : root.height
             - LayoutTokens.sliderTrackInset
             - (height / 2)
        width: root.width + (LayoutTokens.sliderTrackInset * 2)
        height: LayoutTokens.timelineHitHeight
        value: 0.0
        stepSize: 0.001
        showValue: false
        wheelEnabled: false
        enabled: root.timelineEnabled
        accessibleName: qsTr("Timeline")
        accessibleDescription: qsTr("Seek within the current media")

        onInteractionStarted: {
            if (root.viewModel !== null) {
                root.viewModel.beginScrub(timelineSlider.normalizedValue)
            }
        }
        onValueEdited: function(nextValue) {
            if (root.viewModel !== null) {
                root.viewModel.updateScrub(nextValue)
            }
        }
        onInteractionFinished: function(nextValue) {
            if (root.viewModel !== null) {
                root.viewModel.commitScrub(nextValue)
            }
            Qt.callLater(root.syncSliderFromViewModel)
        }
        onInteractionCanceled: {
            if (root.viewModel !== null) {
                root.viewModel.cancelScrub()
            }
            Qt.callLater(root.syncSliderFromViewModel)
        }
    }

    Text {
        id: currentTime

        objectName: "timelineCurrentTime"
        anchors.left: parent.left
        anchors.top: parent.top
        z: 2
        text: root.viewModel !== null
              ? root.viewModel.positionText
              : "--:--:--"
        font: root.currentTimeFont
        color: root.timelineEnabled
               ? ColorTokens.textEmphasis
               : ColorTokens.textMuted
    }

    Text {
        id: durationTime

        objectName: "timelineDurationTime"
        anchors.right: parent.right
        anchors.top: parent.top
        z: 2
        text: root.viewModel !== null
              ? root.viewModel.durationText
              : "--:--:--"
        font: root.durationTimeFont
        color: root.viewModel !== null && root.viewModel.durationSeconds > 0
               ? ColorTokens.textSecondary
               : ColorTokens.textMuted
        horizontalAlignment: Text.AlignRight
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
