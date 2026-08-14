import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives

ButtonBase {
    id: root

    enum Emphasis {
        Secondary,
        Primary
    }

    property string iconId: ""
    property int emphasis: IconButton.Secondary
    property real opticalOffsetX: 0.0
    property int iconSizeOverride: 0

    readonly property bool primary: emphasis === IconButton.Primary
    readonly property int visualIconSize: root.iconSizeOverride > 0
                                          ? root.iconSizeOverride
                                          : primary
                                            ? LayoutTokens.playbackIcon
                                            : LayoutTokens.controlIcon
    readonly property int controlRadius: primary
                                         ? RadiusTokens.controlPlayback
                                         : RadiusTokens.controlTransportSecondary
    readonly property real surfaceAlpha: !primary
                                         ? 0.0
                                         : pressed
                                           ? MaterialTokens.transportPrimaryPressedAlpha
                                           : hovered
                                             ? MaterialTokens.transportPrimaryHoverAlpha
                                             : MaterialTokens.transportPrimaryRestAlpha
    readonly property real iconOpacity: primary ? 1.0 : interactionOpacity

    implicitWidth: primary
                   ? LayoutTokens.playbackControl
                   : LayoutTokens.controlHitMinimum
    implicitHeight: implicitWidth
    width: implicitWidth
    height: implicitHeight
    opacity: primary && !enabled ? OpacityTokens.controlDisabled : 1.0

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    Rectangle {
        id: surface

        objectName: "buttonFocusRing"
        anchors.fill: parent
        radius: root.controlRadius
        color: root.primary
               ? root.withAlpha(ColorTokens.surfaceGlassStrong, root.surfaceAlpha)
               : "transparent"
        border.width: root.activeFocus || root.primary ? 1 : 0
        border.color: root.activeFocus
                      ? root.withAlpha(ColorTokens.focusRing, OpacityTokens.focusRing)
                      : root.withAlpha(ColorTokens.borderGlass,
                                       MaterialTokens.borderSoftAlpha)

        Behavior on color {
            ColorAnimation {
                duration: root.stateTransitionDuration
                easing.type: root.stateTransitionEasingType
                easing.bezierCurve: root.stateTransitionBezier
            }
        }
    }

    Icon {
        id: glyph

        width: root.visualIconSize
        height: root.visualIconSize
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: root.opticalOffsetX
        iconId: root.iconId
        color: root.primary ? ColorTokens.iconPrimary : ColorTokens.iconSecondary
        opacity: root.iconOpacity

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
