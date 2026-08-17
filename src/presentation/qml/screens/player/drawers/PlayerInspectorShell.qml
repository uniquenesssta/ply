import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls
import Player.Presentation.Surfaces

Item {
    id: root

    property bool opened: false
    property string title: ""
    property alias bodyContent: bodyHost.data
    property alias footerContent: footerHost.data

    signal closeRequested()

    objectName: "playerInspectorShell"
    opacity: root.opened ? OpacityTokens.visible : OpacityTokens.hidden
    visible: root.opened || opacity > OpacityTokens.hidden
    enabled: root.opened

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b,
                       colorValue.a * alphaValue)
    }

    transform: Translate {
        x: root.opened ? 0 : SpacingTokens.inspectorContent

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

        Item {
            anchors.fill: parent

            Item {
                id: header

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                height: LayoutTokens.headerHeightCompact

                Text {
                    anchors {
                        left: parent.left
                        right: closeButton.left
                        rightMargin: SpacingTokens.controlAdjacent
                        verticalCenter: parent.verticalCenter
                    }
                    text: root.title
                    color: ColorTokens.textStrong
                    font: TypographyTokens.inspectorTitle
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }

                IconButton {
                    id: closeButton

                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    iconId: "close"
                    toolTipText: qsTr("Close inspector")
                    accessibleName: toolTipText
                    accessibleDescription: qsTr("Close the current inspector panel")

                    onClicked: root.closeRequested()
                }
            }

            Item {
                id: bodyHost

                anchors {
                    top: header.bottom
                    left: parent.left
                    right: parent.right
                    bottom: footerSurface.top
                    topMargin: SpacingTokens.inspectorSectionGap
                    bottomMargin: SpacingTokens.inspectorSectionGap
                }
                clip: true
            }

            Rectangle {
                id: footerSurface

                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                height: LayoutTokens.inspectorFooterHeight
                radius: RadiusTokens.inspectorFooter
                color: root.withAlpha(ColorTokens.surfaceGlassSubtle,
                                      MaterialTokens.footerFillAlpha)

                Item {
                    id: footerHost

                    anchors.fill: parent
                    anchors.margins: SpacingTokens.footerContent
                }
            }
        }
    }
}
