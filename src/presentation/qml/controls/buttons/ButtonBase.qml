import QtQuick
import Player.Presentation.Theme

FocusScope {
    id: root

    property bool toggleOnActivate: false
    property bool checked: false
    property string toolTipText: ""

    readonly property bool hovered: pointerArea.containsMouse
    readonly property bool pressed: pointerArea.pressed
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

    Keys.onReleased: function(event) {
        if (!root.enabled || event.isAutoRepeat) {
            return
        }

        if (event.key === Qt.Key_Space
                || event.key === Qt.Key_Enter
                || event.key === Qt.Key_Return) {
            root.activate()
            event.accepted = true
        }
    }
}
