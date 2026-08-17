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
    surfaceColor: ColorTokens.surfaceInspectorFieldDark
    fillAlpha: MaterialTokens.inspectorDarkFieldFillAlpha
    borderColor: ColorTokens.borderInspectorDark
    borderAlpha: MaterialTokens.inspectorDarkFieldBorderAlpha
    cornerRadius: RadiusTokens.controlSearch
    backdropBlurRadius: MaterialTokens.inspectorDarkFieldBlur
    shadowColor: ElevationTokens.inspectorDarkFieldShadowColor
    shadowRadius: ElevationTokens.inspectorDarkFieldShadowRadius
    shadowYOffset: ElevationTokens.inspectorDarkFieldShadowYOffset
    shadowAlpha: ElevationTokens.inspectorDarkFieldShadowAlpha
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
        color: ColorTokens.textInspectorMuted
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
        color: ColorTokens.textInspectorStrong
        selectionColor: ColorTokens.accentInspector
        selectedTextColor: ColorTokens.textInspectorPrimary
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
        color: ColorTokens.textInspectorMuted
        font: TypographyTokens.controlBody
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
