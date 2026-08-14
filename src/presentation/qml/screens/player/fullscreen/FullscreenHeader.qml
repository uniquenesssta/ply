import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property string mediaTitle: ""
    property bool active: false

    objectName: "playerFullscreenHeader"
    visible: root.active && root.mediaTitle.trim().length > 0
    implicitWidth: LayoutTokens.fullscreenHeaderWidth
    implicitHeight: LayoutTokens.headerHeightCompact
    surfaceColor: ColorTokens.surfaceGlassSubtle
    cornerRadius: RadiusTokens.headerCompact
    fillAlpha: MaterialTokens.headerCompactFillAlpha
    backdropBlurRadius: MaterialTokens.headerCompactBlur
    contentPadding: 0

    TitleText {
        anchors {
            left: parent.left
            right: escapeHint.left
            verticalCenter: parent.verticalCenter
            leftMargin: SpacingTokens.surfacePaddingLarge
            rightMargin: SpacingTokens.controlAdjacent
        }
        text: root.mediaTitle
        variant: TitleText.MediaCompact
    }

    Text {
        id: escapeHint

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: SpacingTokens.surfacePaddingLarge * 2
        }
        text: qsTr("ESC")
        font: TypographyTokens.keycapCompact
        color: ColorTokens.textSecondary
        textFormat: Text.PlainText
        wrapMode: Text.NoWrap
    }
}
