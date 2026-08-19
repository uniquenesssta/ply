import QtQuick

Item {
    id: root

    property bool interactionEnabled: true

    signal toggleFullscreenRequested()

    objectName: "playerFullscreenGestureLayer"

    TapHandler {
        enabled: root.interactionEnabled
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.DragThreshold

        onDoubleTapped: root.toggleFullscreenRequested()
    }
}
