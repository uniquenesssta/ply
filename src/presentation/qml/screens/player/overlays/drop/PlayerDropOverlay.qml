import QtQuick
import Player.Presentation.Feedback

Item {
    id: root

    property var mediaDropHandler: null
    property bool supportedDrag: false

    readonly property bool dragActive: dropArea.containsDrag

    objectName: "playerDropOverlay"

    DropArea {
        id: dropArea
        objectName: "playerMediaDropArea"
        anchors.fill: parent

        onEntered: function(drag) {
            root.supportedDrag = drag.hasUrls
                                 && root.mediaDropHandler !== null
                                 && root.mediaDropHandler.canHandle(drag.urls)
            drag.accepted = root.supportedDrag
        }

        onExited: root.supportedDrag = false

        onDropped: function(drop) {
            const handled = drop.hasUrls
                            && root.mediaDropHandler !== null
                            && root.mediaDropHandler.handleDrop(drop.urls)
            if (handled) {
                drop.acceptProposedAction()
            } else {
                drop.accepted = false
            }
            root.supportedDrag = false
        }
    }

    EmptyFeedback {
        objectName: "playerMediaDropFeedback"
        anchors.centerIn: parent
        visible: dropArea.containsDrag
        title: root.supportedDrag
               ? qsTr("Drop media here")
               : qsTr("This drop cannot be opened")
        detail: root.supportedDrag
                ? qsTr("Release to process the dropped media")
                : qsTr("Use local media files; folders and unsupported data are rejected")
    }
}
