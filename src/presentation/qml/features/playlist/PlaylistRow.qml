import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Controls

Item {
    id: root

    required property var entryId
    required property string displayTitle
    required property string sourceLocation
    required property bool current

    property string indexText: ""
    property bool selected: false
    readonly property bool hovered: rowHover.hovered

    signal selectionRequested(var entryId)
    signal activationRequested(var entryId)
    signal removeRequested(var entryId)

    objectName: "playlistRow"
    height: LayoutTokens.listRowHeight
    activeFocusOnTab: true

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b,
                       colorValue.a * alphaValue)
    }

    Rectangle {
        anchors.fill: parent
        radius: RadiusTokens.listRow
        color: root.selected
               ? root.withAlpha(ColorTokens.surfaceSelection,
                                MaterialTokens.selectionFillAlpha)
               : root.hovered
                 ? root.withAlpha(ColorTokens.surfaceGlassSubtle,
                                  MaterialTokens.rowFillAlpha)
                 : root.withAlpha(ColorTokens.surfaceGlassSubtle,
                                  OpacityTokens.hidden)
        border.width: root.activeFocus || root.selected
                      ? LayoutTokens.surfaceBorderWidth
                      : 0
        border.color: root.activeFocus
                      ? root.withAlpha(ColorTokens.focusRing,
                                       OpacityTokens.focusRing)
                      : root.withAlpha(ColorTokens.borderSelection,
                                       MaterialTokens.selectionBorderAlpha)
    }

    Text {
        id: rowIndex

        anchors {
            left: parent.left
            leftMargin: SpacingTokens.inspectorRowInset
            verticalCenter: parent.verticalCenter
        }
        width: SpacingTokens.listRowIndex
        text: root.indexText
        color: root.current ? ColorTokens.accentStrong : ColorTokens.textTertiary
        font: TypographyTokens.technicalMetadata
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    Item {
        anchors {
            left: parent.left
            leftMargin: SpacingTokens.listRowContent
            right: removeButton.left
            rightMargin: SpacingTokens.controlAdjacent
            top: parent.top
            bottom: parent.bottom
        }

        Text {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.verticalCenter
                bottomMargin: SpacingTokens.controlTight / 2
            }
            text: root.displayTitle
            color: root.current ? ColorTokens.accentStrong : ColorTokens.textStrong
            font: TypographyTokens.mediaTitle
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }

        Text {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.verticalCenter
                topMargin: SpacingTokens.controlTight / 2
            }
            text: root.sourceLocation
            color: ColorTokens.textMuted
            font: TypographyTokens.metaBody
            elide: Text.ElideMiddle
            wrapMode: Text.NoWrap
        }
    }

    IconButton {
        id: removeButton

        anchors {
            right: parent.right
            rightMargin: SpacingTokens.controlTight
            verticalCenter: parent.verticalCenter
        }
        iconId: "close"
        enabled: !root.current
        toolTipText: root.current
                     ? qsTr("Current item cannot be removed yet")
                     : qsTr("Remove from Playlist")
        accessibleName: toolTipText
        accessibleDescription: root.current
                               ? qsTr("Deleting the current item is handled by a later playlist policy")
                               : qsTr("Remove this item from the playlist")

        onClicked: root.removeRequested(root.entryId)
    }

    HoverHandler {
        id: rowHover
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: {
            root.forceActiveFocus()
            root.selectionRequested(root.entryId)
        }
        onDoubleTapped: root.activationRequested(root.entryId)
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            root.activationRequested(root.entryId)
            event.accepted = true
        } else if (event.key === Qt.Key_Space) {
            root.selectionRequested(root.entryId)
            event.accepted = true
        }
    }
}
