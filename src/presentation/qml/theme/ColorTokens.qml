pragma Singleton

import QtQuick

QtObject {
    readonly property color surfaceCanvas: ThemeMode.dark
                                           ? ColorPrimitives.neutralNight950
                                           : ColorPrimitives.neutralMist50
    readonly property color surfaceVideo: ThemeMode.dark
                                          ? ColorPrimitives.neutralNight950
                                          : ColorPrimitives.neutralMist100
    readonly property color surfaceGlass: ThemeMode.dark
                                          ? ColorPrimitives.neutralNight900
                                          : ColorPrimitives.neutralWhite
    readonly property color surfaceGlassSubtle: ThemeMode.dark
                                                ? ColorPrimitives.neutralNight900
                                                : ColorPrimitives.neutralWhite
    readonly property color surfaceGlassStrong: ThemeMode.dark
                                                ? ColorPrimitives.neutralNight850
                                                : ColorPrimitives.neutralWhite
    readonly property color surfaceSelection: ThemeMode.dark
                                              ? ColorPrimitives.violetNightSelected
                                              : ColorPrimitives.violet100
    readonly property color surfaceLetterbox: ThemeMode.dark
                                              ? ColorPrimitives.neutralNight950
                                              : ColorPrimitives.neutralInk950
    readonly property color surfaceAudio: ThemeMode.dark
                                          ? ColorPrimitives.neutralNight950
                                          : ColorPrimitives.neutralMist100
    readonly property color surfaceEmpty: ThemeMode.dark
                                          ? ColorPrimitives.neutralNight950
                                          : ColorPrimitives.neutralMist50
    readonly property color surfaceSeekPreview: ThemeMode.dark
                                                ? ColorPrimitives.neutralNight850
                                                : ColorPrimitives.neutralWhite

    readonly property color surfaceInspector: ThemeMode.dark
                                              ? ColorPrimitives.neutralNight950
                                              : ColorPrimitives.neutralWhite
    readonly property color surfaceInspectorSearch: ThemeMode.dark
                                                    ? ColorPrimitives.neutralNight850
                                                    : ColorPrimitives.neutralWhite
    readonly property color surfaceInspectorRow: ThemeMode.dark
                                                 ? ColorPrimitives.neutralNight900
                                                 : ColorPrimitives.neutralWhite
    readonly property color surfaceInspectorSelection: ThemeMode.dark
                                                       ? ColorPrimitives.violetNightSelected
                                                       : ColorPrimitives.violet100

    readonly property color textPrimary: ThemeMode.dark
                                         ? ColorPrimitives.neutralLavender50
                                         : ColorPrimitives.neutralInk950
    readonly property color textStrong: ThemeMode.dark
                                        ? ColorPrimitives.neutralLavender100
                                        : ColorPrimitives.neutralInk900
    readonly property color textEmphasis: ThemeMode.dark
                                          ? ColorPrimitives.neutralLavender100
                                          : ColorPrimitives.neutralInk800
    readonly property color textSecondary: ThemeMode.dark
                                           ? ColorPrimitives.neutralLavender400
                                           : ColorPrimitives.neutralInk700
    readonly property color textMuted: ThemeMode.dark
                                       ? ColorPrimitives.neutralLavender500
                                       : ColorPrimitives.neutralInk600
    readonly property color textTertiary: ThemeMode.dark
                                          ? ColorPrimitives.neutralLavender500
                                          : ColorPrimitives.neutralInk500
    readonly property color textInverse: ThemeMode.dark
                                         ? ColorPrimitives.neutralInk950
                                         : ColorPrimitives.neutralWhite

    readonly property color iconPrimary: ThemeMode.dark
                                         ? ColorPrimitives.neutralLavender50
                                         : ColorPrimitives.neutralInk950
    readonly property color iconSecondary: ThemeMode.dark
                                           ? ColorPrimitives.neutralLavender400
                                           : ColorPrimitives.neutralInk600

    readonly property color borderGlass: ColorPrimitives.neutralWhite
    readonly property color borderSelection: ThemeMode.dark
                                             ? ColorPrimitives.violet300
                                             : ColorPrimitives.violet400
    readonly property color borderFocus: ThemeMode.dark
                                         ? ColorPrimitives.violet300
                                         : ColorPrimitives.violet400

    readonly property color accentPrimary: ThemeMode.dark
                                           ? ColorPrimitives.violet300
                                           : ColorPrimitives.violet600
    readonly property color accentStrong: ThemeMode.dark
                                          ? ColorPrimitives.violet200
                                          : ColorPrimitives.violet700
    readonly property color accentSoft: ThemeMode.dark
                                        ? ColorPrimitives.violetNightSelected
                                        : ColorPrimitives.violet100
    readonly property color accentGlow: ColorPrimitives.violetGlow
    readonly property color accentCyan: ColorPrimitives.cyan400
    readonly property color accentWarm: ColorPrimitives.warm400

    readonly property color selectionBackground: ThemeMode.dark
                                                 ? ColorPrimitives.violetNightSelected
                                                 : ColorPrimitives.violet100
    readonly property color selectionIndicator: ThemeMode.dark
                                                ? ColorPrimitives.violet300
                                                : ColorPrimitives.violet600
    readonly property color focusRing: ThemeMode.dark
                                       ? ColorPrimitives.violet300
                                       : ColorPrimitives.violet400

    readonly property color feedbackInfo: ColorPrimitives.cyan400
    readonly property color feedbackWarning: ColorPrimitives.warm400
    readonly property color feedbackError: ColorPrimitives.warm400
    readonly property color feedbackNeutral: ThemeMode.dark
                                             ? ColorPrimitives.neutralLavender400
                                             : ColorPrimitives.neutralInk700

    readonly property color controlTrack: ThemeMode.dark
                                          ? ColorPrimitives.neutralLavender500
                                          : ColorPrimitives.neutralInk900
    readonly property color controlBuffered: ThemeMode.dark
                                             ? ColorPrimitives.neutralLavender400
                                             : ColorPrimitives.neutralInk600
    readonly property color controlProgress: ThemeMode.dark
                                             ? ColorPrimitives.violet300
                                             : ColorPrimitives.violet600
    readonly property color controlThumb: ColorPrimitives.neutralWhite
    readonly property color controlThumbBorder: ThemeMode.dark
                                                ? ColorPrimitives.violet300
                                                : ColorPrimitives.violet400
    readonly property color controlPreview: ColorPrimitives.cyan400
    readonly property color controlPendingTarget: ThemeMode.dark
                                                  ? ColorPrimitives.violet300
                                                  : ColorPrimitives.violet400
    readonly property color controlChapterMarker: ThemeMode.dark
                                                  ? ColorPrimitives.neutralLavender500
                                                  : ColorPrimitives.neutralInk500
    readonly property color controlDisabled: ThemeMode.dark
                                             ? ColorPrimitives.neutralLavender500
                                             : ColorPrimitives.neutralInk500
}
