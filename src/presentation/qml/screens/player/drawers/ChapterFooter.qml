import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property var navigationViewModel: null

    objectName: "chapterFooter"
    surfaceColor: ColorTokens.surfaceGlassSubtle
    cornerRadius: RadiusTokens.inspectorFooter
    fillAlpha: MaterialTokens.footerFillAlpha
    borderAlpha: MaterialTokens.borderStrongAlpha
    backdropBlurRadius: MaterialTokens.footerBlur
    contentPadding: SpacingTokens.none

    Column {
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            leftMargin: SpacingTokens.footerContent
        }
        spacing: SpacingTokens.chapterFooterTextGap
        width: Math.max(0, parent.width - LayoutTokens.chapterFooterTextReserve)

        BodyText {
            width: parent.width
            text: qsTr("Current Chapter")
            variant: BodyText.Control
            color: ColorTokens.textStrong
            elide: Text.ElideRight
        }

        CaptionText {
            width: parent.width
            text: root.navigationViewModel !== null
                  && root.navigationViewModel.currentPosition > 0
                  ? String(root.navigationViewModel.currentPosition)
                    + " / " + String(root.navigationViewModel.chapterCount)
                    + " · " + root.navigationViewModel.currentChapterTitle
                  : qsTr("No current chapter")
            variant: CaptionText.Meta
            color: ColorTokens.textMuted
            elide: Text.ElideRight
        }
    }

    Row {
        anchors {
            right: parent.right
            rightMargin: SpacingTokens.controlTight
            verticalCenter: parent.verticalCenter
        }
        spacing: SpacingTokens.controlTight

        IconButton {
            iconId: "previous"
            toolTipText: qsTr("Previous Chapter")
            accessibleName: toolTipText
            enabled: root.navigationViewModel !== null
                     && root.navigationViewModel.canSeekPrevious

            onClicked: root.navigationViewModel.requestPreviousChapter()
        }

        IconButton {
            iconId: "next"
            toolTipText: qsTr("Next Chapter")
            accessibleName: toolTipText
            enabled: root.navigationViewModel !== null
                     && root.navigationViewModel.canSeekNext

            onClicked: root.navigationViewModel.requestNextChapter()
        }
    }
}
