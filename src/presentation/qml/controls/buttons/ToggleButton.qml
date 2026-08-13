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

    Item {
        id: content

        readonly property bool hasIcon: root.iconId.length > 0
        readonly property bool hasText: root.text.length > 0
        readonly property int gap: hasIcon && hasText ? SpacingTokens.controlTight : 0

        implicitWidth: (hasIcon ? icon.width : 0)
                       + gap
                       + (hasText ? label.implicitWidth : 0)
        implicitHeight: Math.max(hasIcon ? icon.height : 0,
                                 hasText ? label.implicitHeight : 0)
        width: implicitWidth
        height: implicitHeight
        anchors.centerIn: parent
        opacity: root.contentOpacity

        Icon {
            id: icon

            visible: content.hasIcon
            width: LayoutTokens.controlIcon
            height: LayoutTokens.controlIcon
            x: root.opticalOffsetX
            y: (parent.height - height) / 2
            iconId: root.iconId
            color: root.checked ? ColorTokens.iconPrimary : ColorTokens.iconSecondary
        }

        BodyText {
            id: label

            visible: content.hasText
            x: (content.hasIcon ? icon.width : 0) + content.gap
            y: (parent.height - height) / 2
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
