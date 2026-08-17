pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import Player.Presentation.Theme

Item {
    id: root

    property Item sourceItem: null
    property int blurRadius: 0
    property real cornerRadius: 0
    property real mappingRevision: 0

    readonly property rect mappedSourceRect: {
        if (root.sourceItem === null || !Number.isFinite(root.mappingRevision)) {
            return Qt.rect(0, 0, 0, 0)
        }
        const origin = root.sourceItem.mapFromItem(root, 0, 0)
        return Qt.rect(origin.x, origin.y, root.width, root.height)
    }

    objectName: "surfaceBackdropBlur"
    visible: root.sourceItem !== null
             && root.blurRadius > 0
             && root.width > 0
             && root.height > 0
    enabled: false

    ShaderEffectSource {
        id: backdropCapture

        anchors.fill: parent
        sourceItem: root.sourceItem
        sourceRect: root.mappedSourceRect
        live: true
        hideSource: false
        recursive: false
    }

    Rectangle {
        id: roundedMask

        anchors.fill: parent
        radius: root.cornerRadius
        color: ColorTokens.surfaceGlass
        visible: false
        layer.enabled: true
    }

    MultiEffect {
        anchors.fill: parent
        source: backdropCapture
        autoPaddingEnabled: false
        blurEnabled: true
        blur: 1.0
        blurMax: root.blurRadius
        maskEnabled: root.cornerRadius > 0
        maskSource: roundedMask
    }
}
