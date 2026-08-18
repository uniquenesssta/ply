pragma ComponentBehavior: Bound

import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property var playlistModel: null
    property var playlistController: null
    property var selectedEntryId: null
    property string filterText: ""

    readonly property int itemCount: root.playlistModel !== null
                                     ? root.playlistModel.count
                                     : 0
    readonly property string normalizedFilter: root.filterText.trim().toLowerCase()

    objectName: "playlistContent"

    function matchesFilter(title, sourceLocation) {
        if (root.normalizedFilter.length === 0) {
            return true
        }
        return title.toLowerCase().indexOf(root.normalizedFilter) >= 0
            || sourceLocation.toLowerCase().indexOf(root.normalizedFilter) >= 0
    }

    ListView {
        id: listView

        anchors.fill: parent
        model: root.playlistModel
        clip: true
        spacing: SpacingTokens.none
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        currentIndex: -1

        delegate: Item {
            id: delegateItem

            required property var entryId
            required property string displayTitle
            required property string sourceLocation
            required property bool current
            required property bool pendingLoading
            required property bool unavailable
            required property int index

            readonly property bool matchesCurrentFilter: root.matchesFilter(
                delegateItem.displayTitle,
                delegateItem.sourceLocation)

            width: ListView.view.width
            height: delegateItem.matchesCurrentFilter
                    ? LayoutTokens.listRowHeight
                      + (delegateItem.index + 1 < root.itemCount
                         ? SpacingTokens.listGap
                         : SpacingTokens.none)
                    : SpacingTokens.none
            visible: delegateItem.matchesCurrentFilter

            PlaylistRow {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                height: LayoutTokens.listRowHeight
                entryId: delegateItem.entryId
                displayTitle: delegateItem.displayTitle
                sourceLocation: delegateItem.sourceLocation
                current: delegateItem.current
                pendingLoading: delegateItem.pendingLoading
                unavailable: delegateItem.unavailable
                indexText: delegateItem.index < 9
                           ? "0" + String(delegateItem.index + 1)
                           : String(delegateItem.index + 1)
                selected: root.selectedEntryId !== null
                          && root.selectedEntryId === delegateItem.entryId

                onSelectionRequested: function(requestedEntryId) {
                    root.selectedEntryId = requestedEntryId
                }
                onActivationRequested: function(requestedEntryId) {
                    root.selectedEntryId = requestedEntryId
                    if (root.playlistController !== null) {
                        root.playlistController.selectEntry(requestedEntryId)
                    }
                }
                onRemoveRequested: function(requestedEntryId) {
                    if (root.playlistController !== null
                            && root.playlistController.removeEntry(requestedEntryId)
                            && root.selectedEntryId === requestedEntryId) {
                        root.selectedEntryId = null
                    }
                }
            }

            DragHandler {
                id: reorderDrag

                target: null
                acceptedButtons: Qt.LeftButton
                enabled: root.playlistController !== null
                         && root.normalizedFilter.length === 0

                onActiveChanged: {
                    if (active || root.playlistController === null) {
                        return
                    }

                    const contentPoint = listView.contentItem.mapFromItem(
                        delegateItem,
                        centroid.position.x,
                        centroid.position.y)
                    const targetIndex = listView.indexAt(contentPoint.x, contentPoint.y)
                    if (targetIndex >= 0 && targetIndex !== delegateItem.index) {
                        root.playlistController.moveEntry(
                            delegateItem.entryId,
                            targetIndex)
                    }
                }
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: root.itemCount === 0
        enabled: false

        Column {
            anchors.centerIn: parent
            width: Math.min(parent.width, LayoutTokens.inspectorWidthNarrow)
            spacing: SpacingTokens.controlTight

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Playlist is empty")
                color: ColorTokens.textSecondary
                font: TypographyTokens.mediaTitle
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                width: parent.width
                text: qsTr("Add media to build a queue.")
                color: ColorTokens.textMuted
                font: TypographyTokens.metaBody
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    onItemCountChanged: {
        if (root.itemCount === 0) {
            root.selectedEntryId = null
        }
    }
}
