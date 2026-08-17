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
        title: qsTr("Playlist")

        bodyContent: [
            PlaylistContent {
                anchors.fill: parent
                playlistModel: root.playlistModel
                playlistController: root.playlistController
            }
        ]

        footerContent: [
            PlaylistFooter {
                anchors.fill: parent
                count: root.playlistModel !== null ? root.playlistModel.count : 0

                onAddMediaRequested: root.addMediaRequested()
            }
        ]

        onCloseRequested: root.closeRequested()
    }
}
