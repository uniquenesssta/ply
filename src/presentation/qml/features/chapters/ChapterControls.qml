import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

IconButton {
    id: root

    property bool opened: false
    property bool compact: false

    objectName: "chapterToggleButton"
    implicitWidth: LayoutTokens.controlHitMinimum
    implicitHeight: LayoutTokens.controlHitMinimum
    toolTipText: root.opened ? qsTr("Hide Chapters") : qsTr("Show Chapters")
    accessibleName: toolTipText
    accessibleDescription: qsTr("Toggle the chapter inspector")

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    Rectangle {
        anchors.fill: parent
        radius: RadiusTokens.controlTransportSecondary
        color: root.opened
               ? root.withAlpha(ColorTokens.surfaceInspectorSelection,
                                MaterialTokens.selectionFillAlpha)
               : "transparent"
        border.width: root.activeFocus || root.opened
                      ? LayoutTokens.surfaceBorderWidth
                      : 0
        border.color: root.activeFocus
                      ? root.withAlpha(ColorTokens.focusRing,
                                       OpacityTokens.focusRing)
                      : root.withAlpha(ColorTokens.borderSelection,
                                       MaterialTokens.selectionBorderAlpha)
    }

    ChapterMarkerGlyph {
        anchors.centerIn: parent
        width: root.compact ? LayoutTokens.controlIconCompact : LayoutTokens.controlIcon
        height: width
        color: root.opened
               ? ColorTokens.accentStrong
               : ColorTokens.iconSecondary
        opacity: root.interactionOpacity
    }
}
