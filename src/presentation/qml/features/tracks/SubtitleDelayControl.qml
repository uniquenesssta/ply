import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property var controller: null

    readonly property bool controlAvailable: root.controller !== null
                                             && root.controller.available
    readonly property bool controlPending: root.controller !== null
                                           && root.controller.pending
    readonly property real minimumSeconds: root.controller !== null
                                           ? root.controller.minimumSeconds
                                           : -2.0
    readonly property real maximumSeconds: root.controller !== null
                                           ? root.controller.maximumSeconds
                                           : 2.0
    readonly property real stepSeconds: root.controller !== null
                                        ? root.controller.stepSeconds
                                        : 0.05

    objectName: "subtitleDelayControl"
    implicitWidth: SizePrimitives.size324
    implicitHeight: SizePrimitives.size54
    width: implicitWidth
    height: implicitHeight
    contentPadding: 0
    surfaceColor: ColorTokens.surfaceGlassSubtle
    fillAlpha: MaterialTokens.footerFillAlpha
    borderColor: ColorTokens.borderGlass
    borderAlpha: MaterialTokens.borderSoftAlpha
    cornerRadius: RadiusTokens.inspectorFooter
    backdropBlurRadius: MaterialTokens.footerBlur
    shadowRadius: ElevationTokens.floatingShadowRadius
    shadowYOffset: ElevationTokens.floatingShadowYOffset
    shadowAlpha: ElevationTokens.floatingShadowAlpha
    opacity: root.controlAvailable
             ? OpacityTokens.visible
             : OpacityTokens.controlDisabled

    function formatMilliseconds(milliseconds) {
        if (milliseconds > 0) {
            return "+" + milliseconds + " ms"
        }
        return milliseconds + " ms"
    }

    function normalizedFromSeconds(seconds) {
        const span = root.maximumSeconds - root.minimumSeconds
        if (span <= 0.0) {
            return 0.5
        }
        return Math.max(0.0, Math.min(1.0,
                    (seconds - root.minimumSeconds) / span))
    }

    function secondsFromNormalized(normalized) {
        const span = root.maximumSeconds - root.minimumSeconds
        return root.minimumSeconds
             + Math.max(0.0, Math.min(1.0, normalized)) * span
    }

    function syncSliderFromController() {
        if (delaySlider.pressed) {
            return
        }

        const seconds = root.controller === null
                      ? 0.0
                      : root.controller.pending
                        ? root.controller.pendingTargetSeconds
                        : root.controller.delaySeconds
        const normalized = root.normalizedFromSeconds(seconds)
        if (Math.abs(delaySlider.value - normalized) > 0.0000001) {
            delaySlider.value = normalized
        }
    }

    Column {
        x: SpacingPrimitives.space14
        anchors.verticalCenter: parent.verticalCenter
        width: 78
        spacing: SpacingPrimitives.space2

        BodyText {
            width: parent.width
            text: qsTr("Subtitle delay")
            variant: BodyText.Control
            color: root.controlAvailable
                   ? ColorTokens.textPrimary
                   : ColorTokens.textMuted
        }

        CaptionText {
            width: parent.width
            variant: root.controlPending
                     ? CaptionText.MicroStrong
                     : CaptionText.Technical
            color: root.controlPending
                   ? ColorTokens.controlPendingTarget
                   : ColorTokens.textMuted
            text: !root.controlAvailable
                  ? qsTr("Unavailable")
                  : root.formatMilliseconds(
                        root.controlPending
                        ? root.controller.pendingTargetMilliseconds
                        : root.controller.delayMilliseconds)
                    + (root.controlPending ? qsTr(" · Applying") : "")
        }
    }

    Slider {
        id: delaySlider

        objectName: "subtitleDelaySlider"
        anchors.verticalCenter: parent.verticalCenter
        x: 104
        width: 118
        height: SizePrimitives.size32
        value: 0.5
        stepSize: root.stepSeconds
                  / Math.max(0.000001,
                             root.maximumSeconds - root.minimumSeconds)
        wheelStep: stepSize
        showValue: false
        enabled: root.controlAvailable && !root.controlPending
        accessibleName: qsTr("Subtitle delay")
        accessibleDescription: qsTr("Negative values advance subtitles; positive values delay subtitles")

        onInteractionFinished: function(normalized) {
            if (root.controller === null
                    || !root.controller.setDelaySeconds(
                        root.secondsFromNormalized(normalized))) {
                root.syncSliderFromController()
            }
        }

        onInteractionCanceled: root.syncSliderFromController()
    }

    TextButton {
        id: resetButton

        objectName: "subtitleDelayResetButton"
        anchors {
            right: parent.right
            rightMargin: SpacingPrimitives.space12
            verticalCenter: parent.verticalCenter
        }
        width: SizePrimitives.size32
        height: SizePrimitives.size32
        text: "↺"
        textColor: root.controlAvailable
                   ? ColorTokens.accentStrong
                   : ColorTokens.textMuted
        enabled: root.controlAvailable && !root.controlPending
        toolTipText: qsTr("Reset subtitle delay")
        accessibleName: toolTipText

        onClicked: {
            if (root.controller !== null
                    && !root.controller.resetDelay()) {
                root.syncSliderFromController()
            }
        }
    }

    Connections {
        target: root.controller

        function onStateChanged() {
            root.syncSliderFromController()
        }
    }

    onControllerChanged: root.syncSliderFromController()
    Component.onCompleted: root.syncSliderFromController()
}
