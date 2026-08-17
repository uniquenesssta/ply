import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

Item {
    id: root

    required property var entryId
    required property string displayTitle
    required property string sourceLocation
    required property bool current

    property string indexText: ""
    property bool selected: false
    readonly property bool hovered: rowHover.hovered
    readonly property bool highlighted: root.current || root.selected

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
        color: root.highlighted
               ? root.withAlpha(ColorTokens.surfaceSelection,
                                MaterialTokens.selectionFillAlpha)
               : root.hovered
                 ? root.withAlpha(ColorTokens.surfaceGlassSubtle,
                                  MaterialTokens.rowFillAlpha)
                 : root.withAlpha(ColorTokens.surfaceGlassSubtle,
                                  OpacityTokens.hidden)
        border.width: root.activeFocus || root.highlighted
                      ? LayoutTokens.surfaceBorderWidth
                      : SpacingTokens.none
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
            leftMargin: SpacingTokens.listRowIndex
            verticalCenter: parent.verticalCenter
        }
        width: SpacingTokens.listRowIndex
        text: root.indexText
        color: root.highlighted ? ColorTokens.accentStrong : ColorTokens.textTertiary
        font: TypographyTokens.microStrong
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    Item {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            leftMargin: SpacingTokens.listRowContent
            rightMargin: SpacingTokens.listRowContent
        }

        Text {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: SpacingTokens.listRowTitleTop
            }
            text: root.displayTitle
            color: ColorTokens.textStrong
            font: TypographyTokens.mediaTitleCompact
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }

        Text {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: SpacingTokens.listRowMetaTop
            }
            text: root.sourceLocation
            color: ColorTokens.textTertiary
            font: TypographyTokens.timecodeExtraSmall
            elide: Text.ElideMiddle
            wrapMode: Text.NoWrap
        }
    }

    Rectangle {
        id: playingRail

        objectName: "playlistPlayingRail"
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: SpacingTokens.listRowIndex
        }
        width: LayoutTokens.listPlayingRailWidth
        height: LayoutTokens.controlHitMinimum
        radius: RadiusTokens.track
        color: ColorTokens.selectionIndicator
        visible: root.current
    }

    IconButton {
        id: removeButton

        anchors {
            right: parent.right
            rightMargin: SpacingTokens.controlTight
            verticalCenter: parent.verticalCenter
        }
        opacity: !root.current && (root.hovered || root.activeFocus)
                 ? OpacityTokens.visible
                 : OpacityTokens.hidden
        enabled: !root.current && opacity > OpacityTokens.hidden
        iconId: "close"
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
