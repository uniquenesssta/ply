pragma Singleton

import QtQuick

QtObject {
    readonly property color surfaceCanvas: ColorPrimitives.neutralMist50
    readonly property color surfaceVideo: ColorPrimitives.neutralMist100
    readonly property color surfaceGlass: ColorPrimitives.neutralWhite
    readonly property color surfaceGlassSubtle: ColorPrimitives.neutralWhite
    readonly property color surfaceGlassStrong: ColorPrimitives.neutralWhite
    readonly property color surfaceSelection: ColorPrimitives.violet100
    readonly property color surfaceLetterbox: ColorPrimitives.neutralInk950
    readonly property color surfaceAudio: ColorPrimitives.neutralMist100
    readonly property color surfaceEmpty: ColorPrimitives.neutralMist50
    readonly property color surfaceSeekPreview: ColorPrimitives.neutralWhite

    readonly property color textPrimary: ColorPrimitives.neutralInk950
    readonly property color textStrong: ColorPrimitives.neutralInk900
    readonly property color textEmphasis: ColorPrimitives.neutralInk800
    readonly property color textSecondary: ColorPrimitives.neutralInk700
    readonly property color textMuted: ColorPrimitives.neutralInk600
    readonly property color textTertiary: ColorPrimitives.neutralInk500
    readonly property color textInverse: ColorPrimitives.neutralWhite

    readonly property color iconPrimary: ColorPrimitives.neutralInk950
    readonly property color iconSecondary: ColorPrimitives.neutralInk600

    readonly property color borderGlass: ColorPrimitives.neutralWhite
    readonly property color borderSelection: ColorPrimitives.violet400
    readonly property color borderFocus: ColorPrimitives.violet400

    readonly property color accentPrimary: ColorPrimitives.violet600
    readonly property color accentStrong: ColorPrimitives.violet700
    readonly property color accentSoft: ColorPrimitives.violet100
    readonly property color accentGlow: ColorPrimitives.violetGlow
    readonly property color accentCyan: ColorPrimitives.cyan400
    readonly property color accentWarm: ColorPrimitives.warm400

    readonly property color selectionBackground: ColorPrimitives.violet100
    readonly property color selectionIndicator: ColorPrimitives.violet600
    readonly property color focusRing: ColorPrimitives.violet400

    readonly property color feedbackInfo: ColorPrimitives.cyan400
    readonly property color feedbackWarning: ColorPrimitives.warm400
    readonly property color feedbackError: ColorPrimitives.warm400
    readonly property color feedbackNeutral: ColorPrimitives.neutralInk700

    readonly property color controlTrack: ColorPrimitives.neutralInk900
    readonly property color controlBuffered: ColorPrimitives.neutralInk600
    readonly property color controlProgress: ColorPrimitives.violet600
    readonly property color controlThumb: ColorPrimitives.neutralWhite
    readonly property color controlThumbBorder: ColorPrimitives.violet400
    readonly property color controlPreview: ColorPrimitives.cyan400
    readonly property color controlPendingTarget: ColorPrimitives.violet400
    readonly property color controlChapterMarker: ColorPrimitives.neutralInk500
    readonly property color controlDisabled: ColorPrimitives.neutralInk500
}
