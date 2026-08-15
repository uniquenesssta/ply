import QtQuick
import Player.Presentation.Theme

FocusScope {
    id: root

    objectName: "playerTopRegion"
    z: ZOrderTokens.floatingHeader

    default property alias content: contentHost.data
    readonly property Item contentItem: contentHost
    readonly property bool controlsFocused: root.activeFocus

    Item {
        id: contentHost
        anchors.fill: parent
    }
}
