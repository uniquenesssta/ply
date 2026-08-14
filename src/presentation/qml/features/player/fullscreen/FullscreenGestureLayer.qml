import QtQuick

Item {
    id: root

    signal toggleFullscreenRequested()

    objectName: "playerFullscreenGestureLayer"

    TapHandler {
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.DragThreshold

        onDoubleTapped: root.toggleFullscreenRequested()
    }
}
