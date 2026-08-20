import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property color color: ColorTokens.selectionIndicator
    readonly property real glyphScale: Math.min(
                                           width / LayoutTokens.chapterMarkerGlyphSize,
                                           height / LayoutTokens.chapterMarkerGlyphSize)

    implicitWidth: LayoutTokens.chapterMarkerGlyphSize
    implicitHeight: LayoutTokens.chapterMarkerGlyphSize

    Rectangle {
        x: 5 * root.glyphScale
        y: 9 * root.glyphScale
        width: LayoutTokens.chapterMarkerGlyphBarWidth * root.glyphScale
        height: LayoutTokens.chapterMarkerGlyphBarShortHeight * root.glyphScale
        radius: root.glyphScale
        color: root.color
    }

    Rectangle {
        x: 13 * root.glyphScale
        y: 8 * root.glyphScale
        width: LayoutTokens.chapterMarkerGlyphBarWidth * root.glyphScale
        height: LayoutTokens.chapterMarkerGlyphBarMediumHeight * root.glyphScale
        radius: root.glyphScale
        color: root.color
    }

    Rectangle {
        x: 21 * root.glyphScale
        y: 7 * root.glyphScale
        width: LayoutTokens.chapterMarkerGlyphBarWidth * root.glyphScale
        height: LayoutTokens.chapterMarkerGlyphBarTallHeight * root.glyphScale
        radius: root.glyphScale
        color: root.color
    }
}
