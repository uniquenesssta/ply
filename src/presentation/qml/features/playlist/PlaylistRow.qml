import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

Item {
    id: root

    required property var entryId
    required property string displayTitle
    required property string sourceLocation
    required property bool current
    required property bool pendingLoading
    required property bool unavailable

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
               ? root.withAlpha(ColorTokens.surfaceInspectorSelection,
                                MaterialTokens.selectionFillAlpha)
               : root.hovered
                 ? root.withAlpha(ColorTokens.surfaceInspectorSearch,
                                  MaterialTokens.footerFillAlpha)
                 : root.withAlpha(ColorTokens.surfaceInspectorRow,
                                  MaterialTokens.rowFillAlpha)
        border.width: root.activeFocus || root.selected
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
        color: root.current
               ? ColorTokens.accentStrong
               : root.unavailable
                 ? ColorTokens.feedbackWarning
                 : root.selected
                   ? ColorTokens.accentPrimary
                   : ColorTokens.textTertiary
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
            color: root.unavailable
                   ? ColorTokens.feedbackWarning
                   : ColorTokens.textStrong
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
            color: root.pendingLoading
                   ? ColorTokens.controlPendingTarget
                   : root.unavailable
                     ? ColorTokens.feedbackNeutral
                     : ColorTokens.textTertiary
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
        opacity: removeButton.opacity > OpacityTokens.hidden
                 ? OpacityTokens.hidden
                 : OpacityTokens.visible
        visible: root.current
    }

    Rectangle {
        id: playbackStateIndicator

        objectName: "playlistEntryPlaybackStateIndicator"
        anchors {
            right: playingRail.left
            rightMargin: SpacingTokens.controlTight
            verticalCenter: parent.verticalCenter
        }
        width: LayoutTokens.listPlayingRailWidth * 2
        height: width
        radius: width / 2
        color: root.unavailable
               ? ColorTokens.feedbackError
               : ColorTokens.controlPendingTarget
        opacity: removeButton.opacity > OpacityTokens.hidden
                 ? OpacityTokens.hidden
                 : OpacityTokens.visible
        visible: root.pendingLoading || root.unavailable
    }

    IconButton {
        id: removeButton

        anchors {
            right: parent.right
            rightMargin: SpacingTokens.controlTight
            verticalCenter: parent.verticalCenter
        }
        opacity: root.hovered || root.activeFocus
                 ? OpacityTokens.visible
                 : OpacityTokens.hidden
        enabled: opacity > OpacityTokens.hidden
        iconId: "close"
        toolTipText: qsTr("Remove from Playlist")
        accessibleName: toolTipText
        accessibleDescription: qsTr("Remove this item from the playlist")

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
