pragma Singleton

import QtQuick

QtObject {
    readonly property int windowDefaultWidth: SizePrimitives.size1320
    readonly property int windowDefaultHeight: SizePrimitives.size700
    readonly property int windowMinimumWidth: SizePrimitives.size960
    readonly property int windowMinimumHeight: SizePrimitives.size540
    readonly property int windowReferenceWidth: SizePrimitives.size1320
    readonly property int windowReferenceHeight: SizePrimitives.size700

    readonly property int headerHeight: SizePrimitives.size54
    readonly property int headerHeightCompact: SizePrimitives.size50
    readonly property int headerWidth: SizePrimitives.size500
    readonly property int headerStatusX: SizePrimitives.size360
    readonly property int headerInfoWidth: SizePrimitives.size420
    readonly property int headerInfoWidthCompact: SizePrimitives.size360
    readonly property int headerActionsWidth: SizePrimitives.size110
    readonly property int headerActionGap: SizePrimitives.size3
    readonly property int fullscreenHeaderWidth: SizePrimitives.size440

    readonly property int oscHeight: SizePrimitives.size124
    readonly property int oscHeightCompact: SizePrimitives.size106
    readonly property int oscMaximumWidth: SizePrimitives.size880
    readonly property int oscMaximumWidthInspector: SizePrimitives.size730
    readonly property int oscInspectorCenterOffset: SizePrimitives.size185
    readonly property int oscMaximumWidthCompact: SizePrimitives.size828
    readonly property int oscTimelineLaneHeight: SizePrimitives.size28
    readonly property int oscTimelineLaneHeightCompact: SizePrimitives.size26
    readonly property int oscControlLaneHeight: SizePrimitives.size40

    readonly property int controlIcon: SizePrimitives.size22
    readonly property int controlIconCompact: SizePrimitives.size21
    readonly property int playbackIcon: SizePrimitives.size24
    readonly property int playbackControl: SizePrimitives.size40
    readonly property int controlHitMinimum: SizePrimitives.size32
    readonly property int volumeTrackWidth: SizePrimitives.size100
    readonly property int trackSelectionPopupWidth: SizePrimitives.size280

    readonly property int sliderDefaultWidth: SizePrimitives.size180
    readonly property int sliderReferenceHitWidth: SizePrimitives.size132
    readonly property int sliderHitHeight: SizePrimitives.size16
    readonly property int sliderTrackInset: SizePrimitives.size3
    readonly property int sliderTrackHeight: SizePrimitives.size3
    readonly property int sliderThumbRest: SizePrimitives.size10
    readonly property int sliderThumbHover: SizePrimitives.size12
    readonly property int sliderThumbPressed: SizePrimitives.size14
    readonly property int sliderThumbBorderWidth: SizePrimitives.size1
    readonly property real sliderFocusRingWidth: SizePrimitives.size1_5
    readonly property int sliderValueWidth: SizePrimitives.size36

    readonly property int surfaceBorderWidth: SizePrimitives.size1

    readonly property int inspectorWidth: SizePrimitives.size368
    readonly property int inspectorWidthNarrow: SizePrimitives.size320
    readonly property int inspectorHeight: SizePrimitives.size652
    readonly property int inspectorSearchWidth: SizePrimitives.size324
    readonly property int inspectorSearchHeight: SizePrimitives.size42
    readonly property int inspectorListWidth: SizePrimitives.size332
    readonly property int listRowHeight: SizePrimitives.size58
    readonly property int listPlayingRailWidth: SizePrimitives.size3
    readonly property int inspectorFooterHeight: SizePrimitives.size54

    readonly property int timelineHitHeight: SizePrimitives.size16
    readonly property int timelineChapterMarkerWidth: SizePrimitives.size1
    readonly property int timelineChapterMarkerHeight: SizePrimitives.size7
    readonly property int timelinePreviewBubbleWidth: SizePrimitives.size64
    readonly property int timelinePreviewBubbleHeight: SizePrimitives.size28
}
