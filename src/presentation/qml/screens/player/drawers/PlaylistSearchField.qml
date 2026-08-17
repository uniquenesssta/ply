import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property alias text: input.text

    objectName: "playlistSearchField"
    implicitWidth: LayoutTokens.inspectorSearchWidth
    implicitHeight: LayoutTokens.inspectorSearchHeight
    surfaceColor: ColorTokens.surfaceInspectorSearch
    cornerRadius: RadiusTokens.controlSearch
    fillAlpha: MaterialTokens.fieldFillAlpha
    borderAlpha: MaterialTokens.borderStrongAlpha
    backdropBlurRadius: MaterialTokens.fieldBlur
    contentPadding: SpacingTokens.none

    Icon {
        id: searchIcon

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            leftMargin: SpacingTokens.listRowIndex
        }
        width: LayoutTokens.controlIcon
        height: LayoutTokens.controlIcon
        iconId: "search"
        color: ColorTokens.iconSecondary
    }

    TextInput {
        id: input

        anchors {
            left: searchIcon.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            leftMargin: SpacingTokens.searchIconToText
            rightMargin: SpacingTokens.listRowIndex
        }
        color: ColorTokens.textStrong
        selectionColor: ColorTokens.selectionBackground
        selectedTextColor: ColorTokens.textStrong
        font: TypographyTokens.controlBody
        verticalAlignment: TextInput.AlignVCenter
        clip: true
        activeFocusOnTab: true
        selectByMouse: true
    }

    Text {
        anchors.fill: input
        visible: input.text.length === 0
        text: qsTr("搜索播放列表")
        color: ColorTokens.textMuted
        font: TypographyTokens.controlBody
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
