import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Primitives

Item {
    id: root
    property string title: ""
    property string detail: ""
    property color titleColor: ColorTokens.textPrimary
    property real textWidth: 0
    readonly property Item contentColumn: body

    implicitWidth: body.width
    implicitHeight: body.implicitHeight
    width: implicitWidth
    height: implicitHeight

    Column {
        id: body
        width: root.textWidth > 0 ? root.textWidth : implicitWidth
        spacing: SpacingTokens.controlTight

        TitleText {
            objectName: "feedbackTitle"
            width: root.textWidth > 0 ? root.textWidth : implicitWidth
            text: root.title
            variant: TitleText.Media
            color: root.titleColor
            visible: text.length > 0
        }

        BodyText {
            objectName: "feedbackDetail"
            width: root.textWidth > 0 ? root.textWidth : implicitWidth
            text: root.detail
            variant: BodyText.Supporting
            color: ColorTokens.textSecondary
            visible: text.length > 0
        }
    }
}
