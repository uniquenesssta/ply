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
    surfaceColor: ColorTokens.surfaceInspectorFieldDark
    fillAlpha: MaterialTokens.inspectorDarkFooterFillAlpha
    borderColor: ColorTokens.borderInspectorDark
    borderAlpha: MaterialTokens.inspectorDarkFooterBorderAlpha
    cornerRadius: RadiusTokens.inspectorFooter
    backdropBlurRadius: MaterialTokens.inspectorDarkFooterBlur
    shadowColor: ElevationTokens.inspectorDarkFooterShadowColor
    shadowRadius: ElevationTokens.inspectorDarkFooterShadowRadius
    shadowYOffset: ElevationTokens.inspectorDarkFooterShadowYOffset
    shadowAlpha: ElevationTokens.inspectorDarkFooterShadowAlpha
    contentPadding: SpacingTokens.none

    CaptionText {
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            leftMargin: SpacingTokens.footerContent
        }
        text: qsTr("拖放排序 · 双击播放")
        variant: CaptionText.Meta
        color: ColorTokens.textInspectorSecondary
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
        color: ColorTokens.accentInspectorStrong
        horizontalAlignment: Text.AlignRight
    }
}
