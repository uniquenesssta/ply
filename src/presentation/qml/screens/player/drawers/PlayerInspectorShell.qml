import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Item {
    id: root

    property bool opened: false
    property string title: ""
    property string subtitle: ""
    property Item backdropSource: null
    property real backdropMappingRevision: 0
    property alias searchContent: searchHost.data
    property alias bodyContent: bodyHost.data
    property alias footerContent: footerHost.data

    signal closeRequested()

    objectName: "playerInspectorShell"
    opacity: root.opened ? OpacityTokens.visible : OpacityTokens.hidden
    visible: root.opened || opacity > OpacityTokens.hidden
    enabled: root.opened

    transform: Translate {
        id: shellTranslation

        x: root.opened ? SpacingTokens.none : SpacingTokens.inspectorContent

        Behavior on x {
            NumberAnimation {
                duration: root.opened
                          ? MotionTokens.inspectorOpenDuration
                          : MotionTokens.inspectorCloseDuration
                easing.type: root.opened
                             ? MotionTokens.enterEasingType
                             : MotionTokens.exitEasingType
                easing.bezierCurve: root.opened
                                    ? MotionTokens.enterBezier
                                    : MotionTokens.exitBezier
            }
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: root.opened
                      ? MotionTokens.inspectorOpenDuration
                      : MotionTokens.inspectorCloseDuration
            easing.type: root.opened
                         ? MotionTokens.enterEasingType
                         : MotionTokens.exitEasingType
            easing.bezierCurve: root.opened
                                ? MotionTokens.enterBezier
                                : MotionTokens.exitBezier
        }
    }

    Drawer {
        anchors.fill: parent
        backdropSource: root.backdropSource
        backdropMappingRevision: root.backdropMappingRevision + shellTranslation.x
        contentPadding: SpacingTokens.none

        TitleText {
            anchors {
                left: parent.left
                top: parent.top
                leftMargin: SpacingTokens.inspectorEdge
                topMargin: SpacingTokens.inspectorTitleTop
            }
            width: Math.max(0, parent.width - (SpacingTokens.inspectorEdge * 2))
            text: root.title
            variant: TitleText.Inspector
        }

        CaptionText {
            anchors {
                left: parent.left
                top: parent.top
                leftMargin: SpacingTokens.inspectorEdge
                topMargin: SpacingTokens.inspectorMetaTop
            }
            width: Math.max(0, parent.width - (SpacingTokens.inspectorEdge * 2))
            text: root.subtitle
            variant: CaptionText.Meta
        }

        Item {
            id: searchHost

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                leftMargin: SpacingTokens.inspectorContent
                rightMargin: SpacingTokens.inspectorContent
                topMargin: SpacingTokens.inspectorSearchTop
            }
            height: LayoutTokens.inspectorSearchHeight
        }

        Item {
            id: footerHost

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                leftMargin: SpacingTokens.inspectorContent
                rightMargin: SpacingTokens.inspectorContent
                bottomMargin: SpacingTokens.inspectorFooterBottom
            }
            height: LayoutTokens.inspectorFooterHeight
        }

        Item {
            id: bodyHost

            anchors {
                left: parent.left
                right: parent.right
                top: searchHost.bottom
                bottom: footerHost.top
                leftMargin: SpacingTokens.inspectorRowInset
                rightMargin: SpacingTokens.inspectorRowInset
                topMargin: SpacingTokens.listSearchToFirst
                bottomMargin: SpacingTokens.inspectorEdge
            }
            clip: true
        }
    }
}
