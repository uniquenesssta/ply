pragma Singleton

import QtQuick

QtObject {
    readonly property color surfaceCanvas: ThemeMode.isDark
                                           ? ColorPrimitives.neutralNight950
                                           : ColorPrimitives.neutralMist50
    readonly property color surfaceVideo: ThemeMode.isDark
                                          ? ColorPrimitives.neutralNight950
                                          : ColorPrimitives.neutralMist100
    readonly property color surfaceGlass: ThemeMode.isDark
                                          ? ColorPrimitives.neutralNight900
                                          : ColorPrimitives.neutralWhite
    readonly property color surfaceGlassSubtle: ThemeMode.isDark
                                                ? ColorPrimitives.neutralNight900
                                                : ColorPrimitives.neutralWhite
    readonly property color surfaceGlassStrong: ThemeMode.isDark
                                                ? ColorPrimitives.neutralNight850
                                                : ColorPrimitives.neutralWhite
    readonly property color surfaceSelection: ThemeMode.isDark
                                              ? ColorPrimitives.violetNightSelected
                                              : ColorPrimitives.violet100
    readonly property color surfaceLetterbox: ThemeMode.isDark
                                              ? ColorPrimitives.neutralNight950
                                              : ColorPrimitives.neutralInk950
    readonly property color surfaceAudio: ThemeMode.isDark
                                          ? ColorPrimitives.neutralNight950
                                          : ColorPrimitives.neutralMist100
    readonly property color surfaceEmpty: ThemeMode.isDark
                                          ? ColorPrimitives.neutralNight950
                                          : ColorPrimitives.neutralMist50
    readonly property color surfaceSeekPreview: ThemeMode.isDark
                                                ? ColorPrimitives.neutralNight850
                                                : ColorPrimitives.neutralWhite

    readonly property color surfaceInspector: ThemeMode.isDark
                                              ? ColorPrimitives.neutralNight950
                                              : ColorPrimitives.neutralWhite
    readonly property color surfaceInspectorSearch: ThemeMode.isDark
                                                    ? ColorPrimitives.neutralNight850
                                                    : ColorPrimitives.neutralWhite
    readonly property color surfaceInspectorRow: ThemeMode.isDark
                                                 ? ColorPrimitives.neutralNight900
                                                 : ColorPrimitives.neutralWhite
    readonly property color surfaceInspectorSelection: ThemeMode.isDark
                                                       ? ColorPrimitives.violetNightSelected
                                                       : ColorPrimitives.violet100

    readonly property color textPrimary: ThemeMode.isDark
                                         ? ColorPrimitives.neutralLavender50
                                         : ColorPrimitives.neutralInk950
    readonly property color textStrong: ThemeMode.isDark
                                        ? ColorPrimitives.neutralLavender100
                                        : ColorPrimitives.neutralInk900
    readonly property color textEmphasis: ThemeMode.isDark
                                          ? ColorPrimitives.neutralLavender100
                                          : ColorPrimitives.neutralInk800
    readonly property color textSecondary: ThemeMode.isDark
                                           ? ColorPrimitives.neutralLavender400
                                           : ColorPrimitives.neutralInk700
    readonly property color textMuted: ThemeMode.isDark
                                       ? ColorPrimitives.neutralLavender500
                                       : ColorPrimitives.neutralInk600
    readonly property color textTertiary: ThemeMode.isDark
                                          ? ColorPrimitives.neutralLavender500
                                          : ColorPrimitives.neutralInk500
    readonly property color textInverse: ThemeMode.isDark
                                         ? ColorPrimitives.neutralInk950
                                         : ColorPrimitives.neutralWhite

    readonly property color iconPrimary: ThemeMode.isDark
                                         ? ColorPrimitives.neutralLavender50
                                         : ColorPrimitives.neutralInk950
    readonly property color iconSecondary: ThemeMode.isDark
                                           ? ColorPrimitives.neutralLavender400
                                           : ColorPrimitives.neutralInk600

    readonly property color borderGlass: ColorPrimitives.neutralWhite
    readonly property color borderSelection: ThemeMode.isDark
                                             ? ColorPrimitives.violet300
                                             : ColorPrimitives.violet400
    readonly property color borderFocus: ThemeMode.isDark
                                         ? ColorPrimitives.violet300
                                         : ColorPrimitives.violet400

    readonly property color accentPrimary: ThemeMode.isDark
                                           ? ColorPrimitives.violet300
                                           : ColorPrimitives.violet600
    readonly property color accentStrong: ThemeMode.isDark
                                          ? ColorPrimitives.violet200
                                          : ColorPrimitives.violet700
    readonly property color accentSoft: ThemeMode.isDark
                                        ? ColorPrimitives.violetNightSelected
                                        : ColorPrimitives.violet100
    readonly property color accentGlow: ColorPrimitives.violetGlow
    readonly property color accentCyan: ColorPrimitives.cyan400
    readonly property color accentWarm: ColorPrimitives.warm400

    readonly property color selectionBackground: ThemeMode.isDark
                                                 ? ColorPrimitives.violetNightSelected
                                                 : ColorPrimitives.violet100
    readonly property color selectionIndicator: ThemeMode.isDark
                                                ? ColorPrimitives.violet300
                                                : ColorPrimitives.violet600
    readonly property color focusRing: ThemeMode.isDark
                                       ? ColorPrimitives.violet300
                                       : ColorPrimitives.violet400

    readonly property color feedbackInfo: ColorPrimitives.cyan400
    readonly property color feedbackWarning: ColorPrimitives.warm400
    readonly property color feedbackError: ColorPrimitives.warm400
    readonly property color feedbackNeutral: ThemeMode.isDark
                                             ? ColorPrimitives.neutralLavender400
                                             : ColorPrimitives.neutralInk700

    readonly property color controlTrack: ThemeMode.isDark
                                          ? ColorPrimitives.neutralLavender500
                                          : ColorPrimitives.neutralInk900
    readonly property color controlBuffered: ThemeMode.isDark
                                             ? ColorPrimitives.neutralLavender400
                                             : ColorPrimitives.neutralInk600
    readonly property color controlProgress: ThemeMode.isDark
                                             ? ColorPrimitives.violet300
                                             : ColorPrimitives.violet600
    readonly property color controlThumb: ColorPrimitives.neutralWhite
    readonly property color controlThumbBorder: ThemeMode.isDark
                                                ? ColorPrimitives.violet300
                                                : ColorPrimitives.violet400
    readonly property color controlPreview: ColorPrimitives.cyan400
    readonly property color controlPendingTarget: ThemeMode.isDark
                                                  ? ColorPrimitives.violet300
                                                  : ColorPrimitives.violet400
    readonly property color controlChapterMarker: ThemeMode.isDark
                                                  ? ColorPrimitives.neutralLavender500
                                                  : ColorPrimitives.neutralInk500
    readonly property color controlDisabled: ThemeMode.isDark
                                             ? ColorPrimitives.neutralLavender500
                                             : ColorPrimitives.neutralInk500
}
