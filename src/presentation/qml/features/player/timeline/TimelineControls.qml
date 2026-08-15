import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    id: root

    property var viewModel: null
    property bool compact: false
    property bool windowActive: true

    readonly property bool timelineEnabled: root.viewModel !== null
                                            && root.viewModel.canSeek
    readonly property string interactionState: !root.timelineEnabled
                                               ? "Disabled"
                                               : root.viewModel.isScrubbing
                                                 ? "Scrubbing"
                                                 : root.viewModel.seekPending
                                                   ? "CommitPending"
                                                   : timelineSlider.hovered
                                                     ? "Hovering"
                                                     : "Idle"
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

    function syncSliderFromViewModel(force) {
        if (timelineSlider.pressed && !force) {
            return
        }

        const normalized = root.viewModel !== null
                         ? root.viewModel.displayedNormalized
                         : 0.0
        if (Math.abs(timelineSlider.value - normalized) > 0.0000001) {
            timelineSlider.value = normalized
        }
    }

    function requestRelativeSeek(direction) {
        if (!root.timelineEnabled || root.viewModel === null || direction === 0) {
            return false
        }

        return root.viewModel.requestRelativeSeek(
            direction * root.viewModel.relativeSeekStepSeconds)
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Down) {
            if (root.requestRelativeSeek(-1)) {
                event.accepted = true
            }
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_Up) {
            if (root.requestRelativeSeek(1)) {
                event.accepted = true
            }
        }
    }

    Slider {
        id: timelineSlider

        objectName: "timelineSlider"
        x: -LayoutTokens.sliderTrackInset
        y: root.compact
           ? ((root.height - height) / 2)
             + LayoutTokens.sliderTrackInset
             - (LayoutTokens.sliderThumbBorderWidth / 2)
           : root.height
             - LayoutTokens.sliderTrackInset
             - (height / 2)
             + (LayoutTokens.sliderThumbBorderWidth / 2)
        width: root.width + (LayoutTokens.sliderTrackInset * 2)
        height: LayoutTokens.timelineHitHeight
        value: 0.0
        stepSize: 0.001
        showValue: false
        keyboardEnabled: false
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

    WheelHandler {
        id: relativeSeekWheel

        target: null
        enabled: root.timelineEnabled
        onWheel: function(event) {
            let delta = event.angleDelta.y
            if (delta === 0) {
                delta = event.pixelDelta.y
            }
            if (event.inverted) {
                delta = -delta
            }
            if (delta === 0) {
                return
            }

            if (root.requestRelativeSeek(delta > 0 ? 1 : -1)) {
                event.accepted = true
            }
        }
    }

    Text {
        id: currentTime

        objectName: "timelineCurrentTime"
        anchors.left: parent.left
        anchors.top: parent.top
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
            root.syncSliderFromViewModel(false)
        }
    }

    onWindowActiveChanged: {
        if (!root.windowActive
                && root.viewModel !== null
                && root.viewModel.isScrubbing) {
            root.viewModel.cancelScrub()
            root.syncSliderFromViewModel(true)
        }
    }
    onViewModelChanged: root.syncSliderFromViewModel(false)
    Component.onCompleted: root.syncSliderFromViewModel(false)
}
