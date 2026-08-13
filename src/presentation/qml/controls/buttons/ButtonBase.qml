import QtQuick
import Player.Presentation.Theme

FocusScope {
    id: root

    property bool toggleOnActivate: false
    property bool checked: false
    property string toolTipText: ""
    property string accessibleName: toolTipText
    property string accessibleDescription: ""
    property bool _keyboardPressed: false

    readonly property bool hovered: pointerArea.containsMouse
    readonly property bool pressed: pointerArea.pressed || _keyboardPressed
    readonly property bool interactionActive: hovered || activeFocus
    readonly property real interactionOpacity: !enabled
                                               ? OpacityTokens.controlDisabled
                                               : pressed
                                                 ? OpacityTokens.controlPressed
                                                 : interactionActive
                                                   ? OpacityTokens.controlHover
                                                   : OpacityTokens.controlIdle
    readonly property int stateTransitionDuration: pressed
                                                   ? MotionTokens.controlPressDuration
                                                   : MotionTokens.controlStateDuration
    readonly property int stateTransitionEasingType: MotionTokens.controlStateEasingType
    readonly property var stateTransitionBezier: MotionTokens.controlStateBezier
    readonly property bool toolTipVisible: enabled
                                           && toolTipText.length > 0
                                           && (hovered || activeFocus)

    signal clicked()
    signal toggled(bool checked)

    activeFocusOnTab: enabled

    Accessible.role: Accessible.Button
    Accessible.name: root.accessibleName
    Accessible.description: root.accessibleDescription
    Accessible.focusable: root.enabled
    Accessible.pressed: root.pressed
    Accessible.checkable: root.toggleOnActivate
    Accessible.checked: root.checked
    Accessible.onPressAction: root.activate()

    function isActivationKey(key) {
        return key === Qt.Key_Space
                || key === Qt.Key_Enter
                || key === Qt.Key_Return
    }

    function activate() {
        if (!enabled) {
            return
        }

        if (toggleOnActivate) {
            checked = !checked
            toggled(checked)
        }
        clicked()
    }

    onActiveFocusChanged: {
        if (!activeFocus) {
            _keyboardPressed = false
        }
    }

    MouseArea {
        id: pointerArea

        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

        onPressed: root.forceActiveFocus(Qt.MouseFocusReason)
        onClicked: root.activate()
    }

    Keys.onPressed: function(event) {
        if (!root.enabled || event.isAutoRepeat || !root.isActivationKey(event.key)) {
            return
        }

        root._keyboardPressed = true
        event.accepted = true
    }

    Keys.onReleased: function(event) {
        if (!root.enabled || event.isAutoRepeat || !root.isActivationKey(event.key)) {
            return
        }

        const shouldActivate = root._keyboardPressed
        root._keyboardPressed = false
        if (shouldActivate) {
            root.activate()
        }
        event.accepted = true
    }
}
