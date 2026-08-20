pragma Singleton

import QtQuick

QtObject {
    readonly property int windowPlayer: RadiusPrimitives.r32
    readonly property int windowFullscreen: RadiusPrimitives.r34
    readonly property int windowMaximized: RadiusPrimitives.r0

    readonly property int surfaceOsc: RadiusPrimitives.r34
    readonly property int surfaceOscCompact: RadiusPrimitives.r32
    readonly property int surfaceInspector: RadiusPrimitives.r32
    readonly property int surfacePopover: RadiusPrimitives.r20
    readonly property int surfaceToast: RadiusPrimitives.r22
    readonly property int surfaceHud: RadiusPrimitives.r24
    readonly property int surfaceDialog: RadiusPrimitives.r30

    readonly property int header: RadiusPrimitives.r27
    readonly property int headerCompact: RadiusPrimitives.r25
    readonly property int controlPlayback: RadiusPrimitives.r20
    readonly property int controlSearch: RadiusPrimitives.r21
    readonly property int controlTransportSecondary: RadiusPrimitives.r16
    readonly property int controlSlider: RadiusPrimitives.r16
    readonly property int listRow: RadiusPrimitives.r18
    readonly property int chapterIndexBadge: RadiusPrimitives.r10
    readonly property int track: RadiusPrimitives.r2
    readonly property int inspectorFooter: RadiusPrimitives.r25
    readonly property int timelinePreviewBubble: RadiusPrimitives.r14
}
