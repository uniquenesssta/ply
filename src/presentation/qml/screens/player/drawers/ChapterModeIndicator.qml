import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property int count: 0

    objectName: "chapterModeIndicator"
    surfaceColor: ColorTokens.surfaceInspectorSearch
    cornerRadius: RadiusTokens.controlSearch
    fillAlpha: MaterialTokens.fieldFillAlpha
    borderAlpha: MaterialTokens.selectionBorderAlpha
    backdropBlurRadius: MaterialTokens.fieldBlur
    contentPadding: SpacingTokens.none

    ChapterMarkerGlyph {
        id: marker

        anchors {
            left: parent.left
            leftMargin: SpacingTokens.listRowIndex
            verticalCenter: parent.verticalCenter
        }
        width: LayoutTokens.controlIcon
        height: width
        color: ColorTokens.accentStrong
    }

    BodyText {
        anchors {
            left: marker.right
            verticalCenter: parent.verticalCenter
            leftMargin: SpacingTokens.controlTight
        }
        text: qsTr("Chapters")
        variant: BodyText.Control
        color: ColorTokens.accentStrong
    }

    CaptionText {
        anchors {
            right: parent.right
            rightMargin: SpacingTokens.listRowIndex
            verticalCenter: parent.verticalCenter
        }
        text: String(root.count)
        variant: CaptionText.CompactStrong
        color: ColorTokens.accentStrong
    }
}
