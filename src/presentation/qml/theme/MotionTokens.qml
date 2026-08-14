pragma Singleton

import QtQuick

QtObject {
    property bool reduceMotionEnabled: false

    readonly property int oscShowDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.shortDuration
    readonly property int oscHideDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int inspectorOpenDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.deliberate
    readonly property int inspectorCloseDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.close
    readonly property int popoverOpenDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.shortDuration
    readonly property int popoverCloseDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int hudShowDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.shortDuration
    readonly property int hudHideDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int dialogOpenDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.deliberate
    readonly property int dialogCloseDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.close
    readonly property int toastShowDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.standard
    readonly property int toastHideDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.shortDuration

    readonly property int timelinePreviewShowDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int timelinePreviewHideDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int timelineScrubUpdateDuration: MotionPrimitives.instant
    readonly property int timelinePendingEnterDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.shortDuration
    readonly property int timelineCommitDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.shortDuration
    readonly property int timelineCancelDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int controlStateDuration: reduceMotionEnabled ? MotionPrimitives.instant : MotionPrimitives.fast
    readonly property int controlPressDuration: MotionPrimitives.instant

    // Semantic inactivity delays are not transition durations and remain intact
    // when Reduce Motion is enabled.
    readonly property int oscHideDelay: 2200
    readonly property int oscFullscreenHideDelay: 1600

    readonly property int enterEasingType: reduceMotionEnabled ? Easing.Linear : Easing.BezierSpline
    readonly property int exitEasingType: reduceMotionEnabled ? Easing.Linear : Easing.BezierSpline
    readonly property int timelineEasingType: reduceMotionEnabled ? Easing.Linear : Easing.BezierSpline
    readonly property int timelineScrubEasingType: Easing.Linear
    readonly property int controlStateEasingType: reduceMotionEnabled ? Easing.Linear : Easing.BezierSpline

    readonly property var enterBezier: reduceMotionEnabled ? [] : MotionPrimitives.easingOutBezier
    readonly property var exitBezier: reduceMotionEnabled ? [] : MotionPrimitives.easingInBezier
    readonly property var timelineBezier: reduceMotionEnabled ? [] : MotionPrimitives.easingOutBezier
    readonly property var controlStateBezier: reduceMotionEnabled ? [] : MotionPrimitives.easingOutBezier
}
