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
    property real shadowXOffset: 0.0
    property real shadowYOffset: ElevationTokens.floatingShadowYOffset
    property real shadowAlpha: ElevationTokens.floatingShadowAlpha
    property int contentPadding: SpacingTokens.surfacePadding

    readonly property real effectiveShadowOpacity: {
        const fillSourceAlpha = root.surfaceColor.a * root.fillAlpha
        const borderSourceAlpha = root.borderColor.a * root.borderAlpha
        const sourceAlpha = fillSourceAlpha > 0.0 ? fillSourceAlpha : borderSourceAlpha
        return sourceAlpha > 0.0
            ? Math.min(1.0, root.shadowAlpha / sourceAlpha)
            : 0.0
    }

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
        shadowHorizontalOffset: root.shadowXOffset
        shadowVerticalOffset: root.shadowYOffset
        shadowColor: root.shadowColor
        shadowOpacity: root.effectiveShadowOpacity
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
