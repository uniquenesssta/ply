import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives

Item {
    id: root

    property real value: 0.0
    property real stepSize: 0.01
    property real wheelStep: stepSize
    property bool showValue: true
    property bool wheelEnabled: true
    property string valueText: Math.round(normalizedValue * 100) + "%"

    readonly property real normalizedValue: clampValue(value)
    readonly property bool hovered: pointerArea.containsMouse
    readonly property bool pressed: pointerArea.pressed
    readonly property real hitTargetWidth: showValue
                                           ? Math.max(0, width
                                                          - SpacingTokens.sliderValueGap
                                                          - LayoutTokens.sliderValueWidth)
                                           : width
    readonly property real trackWidth: Math.max(
                                           0,
                                           hitTargetWidth
                                           - (LayoutTokens.sliderTrackInset * 2))
    readonly property real progressWidth: trackWidth * normalizedValue
    readonly property real visualThumbSize: pressed
                                             ? LayoutTokens.sliderThumbPressed
                                             : hovered
                                               ? LayoutTokens.sliderThumbHover
                                               : LayoutTokens.sliderThumbRest
    readonly property int stateTransitionDuration: pressed
                                                   ? MotionTokens.controlPressDuration
                                                   : MotionTokens.controlStateDuration
    readonly property int stateTransitionEasingType: MotionTokens.controlStateEasingType
    readonly property var stateTransitionBezier: MotionTokens.controlStateBezier
    readonly property real interactionOpacity: enabled
                                               ? OpacityTokens.visible
                                               : OpacityTokens.controlDisabled

    signal interactionStarted()
    signal valueEdited(real value)
    signal interactionFinished(real value)
    signal interactionCanceled(real value)

    implicitWidth: LayoutTokens.sliderDefaultWidth
    implicitHeight: LayoutTokens.controlHitMinimum
    width: implicitWidth
    height: implicitHeight
    activeFocusOnTab: enabled
    opacity: interactionOpacity

    function clampValue(candidate) {
        if (!isFinite(candidate)) {
            return 0.0
        }
        return Math.max(0.0, Math.min(1.0, candidate))
    }

    function quantizedValue(candidate, quantum) {
        const clamped = clampValue(candidate)
        if (!isFinite(quantum) || quantum <= 0.0) {
            return clamped
        }
        return clampValue(Math.round(clamped / quantum) * quantum)
    }

    function applyInteractionValue(candidate, quantum) {
        const nextValue = quantizedValue(candidate, quantum)
        if (Math.abs(nextValue - normalizedValue) <= 0.0000001) {
            return false
        }

        value = nextValue
        valueEdited(nextValue)
        return true
    }

    function updateFromPointer(localX) {
        if (trackWidth <= 0.0) {
            return
        }

        const candidate = (localX - LayoutTokens.sliderTrackInset) / trackWidth
        applyInteractionValue(candidate, stepSize)
    }

    function nudge(delta) {
        if (!enabled) {
            return
        }

        interactionStarted()
        applyInteractionValue(normalizedValue + delta, stepSize)
        interactionFinished(normalizedValue)
    }

    function handleWheel(event) {
        if (!enabled || !wheelEnabled) {
            return
        }

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

        const increment = wheelStep > 0.0 ? wheelStep : stepSize
        nudge(delta > 0 ? increment : -increment)
        event.accepted = true
    }

    onValueChanged: {
        const clamped = clampValue(value)
        if (Math.abs(clamped - value) > 0.0000001 || !isFinite(value)) {
            value = clamped
        }
    }

    Keys.onPressed: function(event) {
        if (!root.enabled) {
            return
        }

        if (event.key === Qt.Key_Left || event.key === Qt.Key_Down) {
            root.nudge(-root.stepSize)
            event.accepted = true
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_Up) {
            root.nudge(root.stepSize)
            event.accepted = true
        }
    }

    Rectangle {
        id: focusRing

        objectName: "sliderFocusRing"
        anchors.fill: parent
        radius: RadiusTokens.controlSlider
        color: "transparent"
        border.width: root.activeFocus ? LayoutTokens.sliderFocusRingWidth : 0
        border.color: ColorTokens.focusRing
        opacity: root.activeFocus ? OpacityTokens.focusRing : OpacityTokens.hidden

        Behavior on opacity {
            NumberAnimation {
                duration: root.stateTransitionDuration
                easing.type: root.stateTransitionEasingType
                easing.bezierCurve: root.stateTransitionBezier
            }
        }
    }

    Item {
        id: hitTarget

        objectName: "sliderHitTarget"
        x: 0
        y: (root.height - height) / 2
        width: root.hitTargetWidth
        height: LayoutTokens.sliderHitHeight

        Rectangle {
            id: track

            objectName: "sliderTrack"
            x: LayoutTokens.sliderTrackInset
            y: (parent.height - height) / 2
            width: root.trackWidth
            height: LayoutTokens.sliderTrackHeight
            radius: height / 2
            color: ColorTokens.controlTrack
        }

        Rectangle {
            id: progress

            objectName: "sliderProgress"
            x: track.x
            y: track.y
            width: root.progressWidth
            height: track.height
            radius: height / 2
            color: ColorTokens.controlProgress
        }

        Rectangle {
            id: thumb

            objectName: "sliderThumb"
            width: root.visualThumbSize
            height: width
            x: track.x + (track.width * root.normalizedValue) - (width / 2)
            y: (parent.height - height) / 2
            radius: width / 2
            color: ColorTokens.controlThumb
            border.width: LayoutTokens.sliderThumbBorderWidth
            border.color: ColorTokens.controlThumbBorder

            Behavior on width {
                NumberAnimation {
                    duration: root.stateTransitionDuration
                    easing.type: root.stateTransitionEasingType
                    easing.bezierCurve: root.stateTransitionBezier
                }
            }

            Behavior on height {
                NumberAnimation {
                    duration: root.stateTransitionDuration
                    easing.type: root.stateTransitionEasingType
                    easing.bezierCurve: root.stateTransitionBezier
                }
            }
        }

        MouseArea {
            id: pointerArea

            anchors.fill: parent
            enabled: root.enabled
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton

            onPressed: function(mouse) {
                root.forceActiveFocus(Qt.MouseFocusReason)
                root.interactionStarted()
                root.updateFromPointer(mouse.x)
            }

            onPositionChanged: function(mouse) {
                if (pressed) {
                    root.updateFromPointer(mouse.x)
                }
            }

            onReleased: function(mouse) {
                root.updateFromPointer(mouse.x)
                root.interactionFinished(root.normalizedValue)
            }

            onCanceled: root.interactionCanceled(root.normalizedValue)
        }
    }

    TimecodeText {
        id: valueLabel

        objectName: "sliderValue"
        visible: root.showValue
        width: LayoutTokens.sliderValueWidth
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        variant: TimecodeText.ExtraSmall
        text: root.valueText
        color: root.enabled ? ColorTokens.textPrimary : ColorTokens.textMuted
        horizontalAlignment: Text.AlignRight
    }

    WheelHandler {
        id: wheelHandler

        target: null
        enabled: root.enabled && root.wheelEnabled
        onWheel: function(event) {
            root.handleWheel(event)
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: root.stateTransitionDuration
            easing.type: root.stateTransitionEasingType
            easing.bezierCurve: root.stateTransitionBezier
        }
    }
}
