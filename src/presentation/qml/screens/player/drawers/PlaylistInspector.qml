import QtQuick

Item {
    id: root

    property bool opened: false
    property var playlistModel: null
    property var playlistController: null

    signal closeRequested()
    signal addMediaRequested()

    objectName: "playlistInspector"

    PlayerInspectorShell {
        anchors.fill: parent
        opened: root.opened
        title: qsTr("播放列表")
        subtitle: qsTr("%1 个媒体").arg(root.playlistModel !== null
                                         ? root.playlistModel.count
                                         : 0)

        searchContent: [
            PlaylistSearchField {
                id: searchField
                anchors.fill: parent
            }
        ]

        bodyContent: [
            PlaylistContent {
                anchors.fill: parent
                playlistModel: root.playlistModel
                playlistController: root.playlistController
                filterText: searchField.text
            }
        ]

        footerContent: [
            PlaylistFooter {
                anchors.fill: parent
                count: root.playlistModel !== null ? root.playlistModel.count : 0
                currentPosition: root.playlistModel !== null
                                 ? root.playlistModel.currentPosition
                                 : 0

                onAddMediaRequested: root.addMediaRequested()
            }
        ]

        onCloseRequested: root.closeRequested()
    }
}
