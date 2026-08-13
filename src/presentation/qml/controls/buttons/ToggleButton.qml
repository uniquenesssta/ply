import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives

ButtonBase {
    id: root

    property string iconId: ""
    property string text: ""
    property real opticalOffsetX: 0.0

    readonly property real contentOpacity: !enabled
                                           ? 1.0
                                           : pressed
                                             ? OpacityTokens.controlPressed
                                             : hovered || activeFocus || checked
                                               ? OpacityTokens.controlHover
                                               : OpacityTokens.controlIdle

    toggleOnActivate: true
    implicitWidth: Math.max(LayoutTokens.controlHitMinimum,
                            content.implicitWidth + (SpacingTokens.controlAdjacent * 2))
    implicitHeight: LayoutTokens.controlHitMinimum
    width: implicitWidth
    height: implicitHeight
    opacity: enabled ? 1.0 : OpacityTokens.controlDisabled

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    Rectangle {
        id: selectionSurface

        anchors.fill: parent
        radius: RadiusTokens.controlTransportSecondary
        color: root.checked
               ? root.withAlpha(ColorTokens.surfaceSelection,
                                MaterialTokens.selectionFillAlpha)
               : "transparent"
        border.width: root.checked || root.activeFocus ? 1 : 0
        border.color: root.activeFocus
                      ? root.withAlpha(ColorTokens.focusRing,
                                       OpacityTokens.focusRing)
                      : root.withAlpha(ColorTokens.borderSelection,
                                       MaterialTokens.selectionBorderAlpha)

        Behavior on color {
            ColorAnimation {
                duration: root.stateTransitionDuration
                easing.type: root.stateTransitionEasingType
                easing.bezierCurve: root.stateTransitionBezier
            }
        }
    }

    Row {
        id: content

        anchors.centerIn: parent
        spacing: icon.visible && label.visible ? SpacingTokens.controlTight : 0
        opacity: root.contentOpacity

        Icon {
            id: icon

            visible: root.iconId.length > 0
            width: LayoutTokens.controlIcon
            height: LayoutTokens.controlIcon
            anchors.verticalCenter: parent.verticalCenter
            iconId: root.iconId
            color: root.checked ? ColorTokens.iconPrimary : ColorTokens.iconSecondary
            transform: Translate { x: root.opticalOffsetX }
        }

        BodyText {
            id: label

            visible: root.text.length > 0
            text: root.text
            variant: BodyText.Control
            color: root.checked ? ColorTokens.textPrimary : ColorTokens.textSecondary
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Behavior on opacity {
            NumberAnimation {
                duration: root.stateTransitionDuration
                easing.type: root.stateTransitionEasingType
                easing.bezierCurve: root.stateTransitionBezier
            }
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
