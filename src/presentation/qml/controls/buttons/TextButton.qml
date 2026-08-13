import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives

ButtonBase {
    id: root

    property string text: ""
    property color textColor: ColorTokens.textPrimary

    accessibleName: text.length > 0 ? text : toolTipText

    implicitWidth: Math.max(LayoutTokens.controlHitMinimum,
                            label.implicitWidth + (SpacingTokens.controlAdjacent * 2))
    implicitHeight: LayoutTokens.controlHitMinimum
    width: implicitWidth
    height: implicitHeight

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    Rectangle {
        objectName: "buttonFocusRing"
        anchors.fill: parent
        radius: RadiusTokens.controlTransportSecondary
        color: "transparent"
        border.width: root.activeFocus ? 1 : 0
        border.color: root.withAlpha(ColorTokens.focusRing,
                                     OpacityTokens.focusRing)
    }

    BodyText {
        id: label

        anchors {
            left: parent.left
            right: parent.right
            leftMargin: SpacingTokens.controlAdjacent
            rightMargin: SpacingTokens.controlAdjacent
            verticalCenter: parent.verticalCenter
        }
        text: root.text
        variant: BodyText.Control
        color: root.textColor
        opacity: root.interactionOpacity
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Behavior on opacity {
            NumberAnimation {
                duration: root.stateTransitionDuration
                easing.type: root.stateTransitionEasingType
                easing.bezierCurve: root.stateTransitionBezier
            }
        }
    }
}
