import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

Rectangle {
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

    objectName: "audioDelayControl"
    implicitWidth: LayoutTokens.delayControlWidth
    implicitHeight: LayoutTokens.delayControlHeight
    width: implicitWidth
    height: implicitHeight
    color: Qt.rgba(
        ColorTokens.surfaceGlassSubtle.r,
        ColorTokens.surfaceGlassSubtle.g,
        ColorTokens.surfaceGlassSubtle.b,
        ColorTokens.surfaceGlassSubtle.a * MaterialTokens.footerFillAlpha)
    border.width: LayoutTokens.surfaceBorderWidth
    border.color: Qt.rgba(
        ColorTokens.borderGlass.r,
        ColorTokens.borderGlass.g,
        ColorTokens.borderGlass.b,
        ColorTokens.borderGlass.a * MaterialTokens.borderSoftAlpha)
    radius: RadiusTokens.inspectorFooter
    z: ZOrderTokens.overlay
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
        width: LayoutTokens.delayControlLabelWidth
        spacing: SpacingPrimitives.space2

        Text {
            width: parent.width
            text: qsTr("Audio delay")
            font: TypographyTokens.controlBody
            color: root.controlAvailable
                   ? ColorTokens.textPrimary
                   : ColorTokens.textMuted
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            font: root.controlPending
                  ? TypographyTokens.microStrong
                  : TypographyTokens.technicalMetadata
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
            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            maximumLineCount: 1
        }
    }

    Slider {
        id: delaySlider

        objectName: "audioDelaySlider"
        anchors.verticalCenter: parent.verticalCenter
        x: LayoutTokens.delayControlSliderX
        width: LayoutTokens.delayControlSliderWidth
        height: LayoutTokens.controlHitMinimum
        value: 0.5
        stepSize: root.stepSeconds
                  / Math.max(0.000001,
                             root.maximumSeconds - root.minimumSeconds)
        wheelStep: stepSize
        showValue: false
        enabled: root.controlAvailable && !root.controlPending
        accessibleName: qsTr("Audio delay")
        accessibleDescription: qsTr("Negative values advance audio; positive values delay audio")

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

        objectName: "audioDelayResetButton"
        anchors {
            right: parent.right
            rightMargin: SpacingPrimitives.space12
            verticalCenter: parent.verticalCenter
        }
        width: LayoutTokens.delayControlResetSize
        height: LayoutTokens.delayControlResetSize
        text: "↺"
        textColor: root.controlAvailable
                   ? ColorTokens.accentStrong
                   : ColorTokens.textMuted
        enabled: root.controlAvailable && !root.controlPending
        toolTipText: qsTr("Reset audio delay")
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
