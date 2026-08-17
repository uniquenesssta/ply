pragma ComponentBehavior: Bound

import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property var playlistModel: null
    property var playlistController: null
    property var selectedEntryId: null

    readonly property int itemCount: root.playlistModel !== null
                                     ? root.playlistModel.count
                                     : 0

    objectName: "playlistContent"

    ListView {
        id: listView

        anchors.fill: parent
        model: root.playlistModel
        clip: true
        spacing: SpacingTokens.listGap
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        currentIndex: -1

        delegate: Item {
            id: delegateItem

            required property var entryId
            required property string displayTitle
            required property string sourceLocation
            required property bool current
            required property int index

            width: ListView.view.width
            height: LayoutTokens.listRowHeight

            PlaylistRow {
                anchors.fill: parent
                entryId: delegateItem.entryId
                displayTitle: delegateItem.displayTitle
                sourceLocation: delegateItem.sourceLocation
                current: delegateItem.current
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
