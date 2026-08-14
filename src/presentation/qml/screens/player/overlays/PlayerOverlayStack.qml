import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    objectName: "playerOverlayStack"
    z: ZOrderTokens.overlay

    default property alias content: contentHost.data
    readonly property Item contentItem: contentHost

    Item {
        id: contentHost
        anchors.fill: parent
    }
}
