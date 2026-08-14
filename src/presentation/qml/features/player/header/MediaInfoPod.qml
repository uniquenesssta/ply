import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property string mediaTitle: ""
    property string metadataText: ""
    property bool compact: false

    readonly property bool metadataVisible: !root.compact
                                             && root.metadataText.trim().length > 0
                                             && root.width >= LayoutTokens.headerInfoWidth
    readonly property int horizontalInset: root.compact
                                            ? SpacingTokens.headerContentCompact
                                            : SpacingTokens.headerContent

    objectName: "playerMediaInfoPod"
    visible: root.mediaTitle.trim().length > 0
    implicitWidth: root.compact
                   ? LayoutTokens.headerInfoWidthCompact
                   : LayoutTokens.headerInfoWidth
    implicitHeight: root.compact
                    ? LayoutTokens.headerHeightCompact
                    : LayoutTokens.headerHeight
    cornerRadius: root.compact
                  ? RadiusTokens.headerCompact
                  : RadiusTokens.header
    fillAlpha: root.compact
               ? MaterialTokens.headerCompactFillAlpha
               : MaterialTokens.headerFillAlpha
    backdropBlurRadius: root.compact
                        ? MaterialTokens.headerCompactBlur
                        : MaterialTokens.headerBlur
    contentPadding: 0

    Column {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: root.horizontalInset
            rightMargin: root.horizontalInset
        }
        spacing: SpacingTokens.headerTitleMeta

        TitleText {
            width: parent.width
            text: root.mediaTitle
            variant: root.compact ? TitleText.MediaCompact : TitleText.Media
        }

        CaptionText {
            width: parent.width
            visible: root.metadataVisible
            text: root.metadataText
            variant: CaptionText.Technical
        }
    }
}
