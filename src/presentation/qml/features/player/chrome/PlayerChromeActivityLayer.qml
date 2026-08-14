import QtQuick

Item {
    id: root

    signal activityDetected(string reason)

    objectName: "playerChromeActivityLayer"

    HoverHandler {
        id: pointerHover

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
