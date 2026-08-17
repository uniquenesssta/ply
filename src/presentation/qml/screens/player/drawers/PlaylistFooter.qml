import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property int count: 0
    property int currentPosition: 0

    signal addMediaRequested()

    objectName: "playlistFooter"
    cornerRadius: RadiusTokens.inspectorFooter
    fillAlpha: MaterialTokens.footerFillAlpha
    borderAlpha: MaterialTokens.borderStrongAlpha
    backdropBlurRadius: MaterialTokens.footerBlur
    contentPadding: SpacingTokens.none

    CaptionText {
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            leftMargin: SpacingTokens.footerContent
        }
        text: qsTr("拖放排序 · 双击播放")
        variant: CaptionText.Meta
        color: ColorTokens.textSecondary
    }

    CaptionText {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: SpacingTokens.footerContent
        }
        text: root.count > 0
              ? (root.currentPosition > 0
                 ? String(root.currentPosition) + " / " + String(root.count)
                 : String(root.count))
              : "0"
        variant: CaptionText.CompactStrong
        color: ColorTokens.accentStrong
        horizontalAlignment: Text.AlignRight
    }
}
