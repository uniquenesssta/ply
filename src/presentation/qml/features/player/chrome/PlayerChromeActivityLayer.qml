import QtQuick

Item {
    id: root

    property bool cursorHidden: false

    readonly property bool pointerInside: pointerHover.hovered

    signal activityDetected(string reason)

    objectName: "playerChromeActivityLayer"

    HoverHandler {
        id: pointerHover

        cursorShape: root.cursorHidden ? Qt.BlankCursor : undefined

        onHoveredChanged: {
            if (hovered) {
                root.activityDetected("pointer-enter")
            }
        }
        onPointChanged: {
            if (hovered) {
                root.activityDetected("pointer-move")
            }
        }
    }
}
