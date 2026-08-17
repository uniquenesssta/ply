pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import Player.Presentation.Theme

Rectangle {
    id: root

    default property alias contentData: contentHost.data

    property Item backdropSource: null
    property real backdropMappingRevision: 0
    property color surfaceColor: ColorTokens.surfaceGlass
    property real fillAlpha: MaterialTokens.inspectorFillAlpha
    property color borderColor: ColorTokens.borderGlass
    property real borderAlpha: MaterialTokens.borderSoftAlpha
    property real borderWidth: LayoutTokens.surfaceBorderWidth
    property real cornerRadius: RadiusTokens.surfaceInspector
    property int backdropBlurRadius: MaterialTokens.inspectorBlur
    property color shadowColor: ElevationTokens.shadowColor
    property int shadowRadius: ElevationTokens.floatingShadowRadius
    property real shadowYOffset: ElevationTokens.floatingShadowYOffset
    property real shadowAlpha: ElevationTokens.floatingShadowAlpha
    property int contentPadding: SpacingTokens.surfacePadding

    color: Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b,
                   surfaceColor.a * fillAlpha)
    radius: cornerRadius
    border.width: borderWidth
    border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b,
                          borderColor.a * borderAlpha)
    z: ZOrderTokens.overlay

    BackdropBlur {
        anchors.fill: parent
        z: -1
        sourceItem: root.backdropSource
        blurRadius: root.backdropBlurRadius
        cornerRadius: root.cornerRadius
        mappingRevision: root.backdropMappingRevision
    }

    layer.enabled: shadowAlpha > 0.0 && shadowRadius > 0
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowBlur: 1.0
        blurMax: root.shadowRadius
        shadowVerticalOffset: root.shadowYOffset
        shadowColor: root.shadowColor
        shadowOpacity: {
            const sourceAlpha = Math.max(root.fillAlpha, root.borderAlpha)
            return sourceAlpha > 0.0
                ? Math.min(1.0, root.shadowAlpha / sourceAlpha)
                : 0.0
        }
    }

    readonly property Item contentItem: Item {
        id: contentHost
        parent: root
        objectName: "surfaceContent"
        anchors.fill: parent
        anchors.margins: root.contentPadding
        clip: true
    }
}
