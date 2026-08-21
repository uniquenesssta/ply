pragma ComponentBehavior: Bound

import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property var chapterModel: null
    property var navigationViewModel: null

    objectName: "chapterContent"

    ListView {
        id: chapterList

        anchors.fill: parent
        visible: root.chapterModel !== null && root.chapterModel.count > 0
        model: root.chapterModel
        spacing: SpacingTokens.listGap
        clip: true
        reuseItems: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: ChapterRow {
            required property var index

            width: chapterList.width
            chapterIndex: index
            current: root.navigationViewModel !== null
                     && root.navigationViewModel.currentChapterIndex === chapterIndex
            pending: root.navigationViewModel !== null
                     && root.navigationViewModel.pendingChapterIndex === chapterIndex
            seekEnabled: root.navigationViewModel !== null
                         && root.navigationViewModel.canSeek

            onSeekRequested: function(requestedIndex) {
                if (root.navigationViewModel !== null) {
                    root.navigationViewModel.requestChapterSeek(requestedIndex)
                }
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: SpacingTokens.controlTight
        visible: root.chapterModel === null || root.chapterModel.count === 0

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: LayoutTokens.chapterEmptyStateSurfaceSize
            height: LayoutTokens.chapterEmptyStateSurfaceSize
            radius: RadiusTokens.listRow
            color: ColorTokens.surfaceInspectorSearch
            border.width: LayoutTokens.surfaceBorderWidth
            border.color: ColorTokens.borderGlass

            ChapterMarkerGlyph {
                anchors.centerIn: parent
                width: LayoutTokens.chapterMarkerGlyphSize
                height: LayoutTokens.chapterMarkerGlyphSize
                color: ColorTokens.iconSecondary
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("This media has no chapters")
            font: TypographyTokens.controlBody
            color: ColorTokens.textStrong
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Chapter metadata will appear here when available")
            font: TypographyTokens.metaBody
            color: ColorTokens.textMuted
        }
    }
}
