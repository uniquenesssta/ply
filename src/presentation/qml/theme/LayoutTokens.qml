pragma Singleton

import QtQuick

QtObject {
    // Preserve the current application window behavior while exposing the final
    // Airy Glass reference geometry separately for later responsive work.
    readonly property int windowDefaultWidth: SizePrimitives.size1280
    readonly property int windowDefaultHeight: SizePrimitives.size720
    readonly property int windowMinimumWidth: SizePrimitives.size960
    readonly property int windowMinimumHeight: SizePrimitives.size540
    readonly property int windowReferenceWidth: SizePrimitives.size1320
    readonly property int windowReferenceHeight: SizePrimitives.size700

    readonly property int headerHeight: SizePrimitives.size54
    readonly property int headerHeightCompact: SizePrimitives.size50
    readonly property int headerInfoWidth: SizePrimitives.size420
    readonly property int headerInfoWidthCompact: SizePrimitives.size360
    readonly property int headerActionsWidth: SizePrimitives.size110

    readonly property int oscHeight: SizePrimitives.size124
    readonly property int oscHeightCompact: SizePrimitives.size106
    readonly property int oscMaximumWidth: SizePrimitives.size880
    readonly property int oscTimelineLaneHeight: SizePrimitives.size28
    readonly property int oscTimelineLaneHeightCompact: SizePrimitives.size26
    readonly property int oscControlLaneHeight: SizePrimitives.size40

    readonly property int controlIcon: SizePrimitives.size22
    readonly property int controlIconCompact: SizePrimitives.size21
    readonly property int playbackControl: SizePrimitives.size40
    readonly property int controlHitMinimum: SizePrimitives.size32
    readonly property int volumeTrackWidth: SizePrimitives.size100

    readonly property int inspectorWidth: SizePrimitives.size368
    readonly property int inspectorWidthNarrow: SizePrimitives.size320
    readonly property int inspectorSearchHeight: SizePrimitives.size42
    readonly property int listRowHeight: SizePrimitives.size58
    readonly property int inspectorFooterHeight: SizePrimitives.size54

    readonly property int timelineHitHeight: SizePrimitives.size16
    readonly property int timelineChapterMarkerWidth: SizePrimitives.size1
    readonly property int timelineChapterMarkerHeight: SizePrimitives.size7
    readonly property int timelinePreviewBubbleWidth: SizePrimitives.size64
    readonly property int timelinePreviewBubbleHeight: SizePrimitives.size28
}
