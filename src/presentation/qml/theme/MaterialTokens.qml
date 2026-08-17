pragma Singleton

import QtQuick

QtObject {
    readonly property int headerBlur: BlurPrimitives.b28
    readonly property int headerCompactBlur: BlurPrimitives.b30
    readonly property int oscBlur: BlurPrimitives.b36
    readonly property int oscCompactBlur: BlurPrimitives.b38
    readonly property int controlBlur: BlurPrimitives.b18
    readonly property int inspectorBlur: BlurPrimitives.b42
    readonly property int fieldBlur: BlurPrimitives.b18
    readonly property int footerBlur: BlurPrimitives.b20
    readonly property int hudBlur: BlurPrimitives.b18
    readonly property int dialogBlur: BlurPrimitives.b42
    readonly property int popoverBlur: BlurPrimitives.b18

    readonly property int atmosphereFieldBlur: BlurPrimitives.b24
    readonly property int atmosphereLavenderBlur: BlurPrimitives.b90
    readonly property int atmosphereWarmBlur: BlurPrimitives.b95
    readonly property int atmosphereCyanBlur: BlurPrimitives.b100

    readonly property real headerFillAlpha: ThemeMode.isDark
                                            ? MaterialAlphaPrimitives.a100
                                            : MaterialAlphaPrimitives.a52
    readonly property real headerCompactFillAlpha: ThemeMode.isDark
                                                   ? MaterialAlphaPrimitives.a100
                                                   : MaterialAlphaPrimitives.a42
    readonly property real oscFillAlpha: ThemeMode.isDark
                                         ? MaterialAlphaPrimitives.a100
                                         : MaterialAlphaPrimitives.a34
    readonly property real oscCompactFillAlpha: ThemeMode.isDark
                                                ? MaterialAlphaPrimitives.a100
                                                : MaterialAlphaPrimitives.a32
    readonly property real controlFillAlpha: ThemeMode.isDark
                                             ? MaterialAlphaPrimitives.a100
                                             : MaterialAlphaPrimitives.a48
    readonly property real inspectorFillAlpha: ThemeMode.isDark
                                               ? MaterialAlphaPrimitives.a100
                                               : MaterialAlphaPrimitives.a38
    readonly property real fieldFillAlpha: ThemeMode.isDark
                                           ? MaterialAlphaPrimitives.a100
                                           : MaterialAlphaPrimitives.a36
    readonly property real footerFillAlpha: ThemeMode.isDark
                                            ? MaterialAlphaPrimitives.a100
                                            : MaterialAlphaPrimitives.a26
    readonly property real rowFillAlpha: ThemeMode.isDark
                                         ? MaterialAlphaPrimitives.a100
                                         : MaterialAlphaPrimitives.a07
    readonly property real selectionFillAlpha: ThemeMode.isDark
                                               ? MaterialAlphaPrimitives.a100
                                               : MaterialAlphaPrimitives.a66
    readonly property real borderSoftAlpha: ThemeMode.isDark
                                            ? MaterialAlphaPrimitives.a100
                                            : MaterialAlphaPrimitives.a48
    readonly property real borderStrongAlpha: MaterialAlphaPrimitives.a100
    readonly property real selectionBorderAlpha: ThemeMode.isDark
                                                 ? MaterialAlphaPrimitives.a100
                                                 : MaterialAlphaPrimitives.a28
    readonly property real popoverFillAlpha: ThemeMode.isDark
                                             ? MaterialAlphaPrimitives.a100
                                             : MaterialAlphaPrimitives.a52

    readonly property real atmosphereVignetteAlpha: MaterialAlphaPrimitives.a03
    readonly property real atmosphereLavenderAlpha: MaterialAlphaPrimitives.a34
    readonly property real atmosphereCyanAlpha: MaterialAlphaPrimitives.a22
    readonly property real atmosphereWarmAlpha: MaterialAlphaPrimitives.a34
    readonly property real windowBorderAlpha: ThemeMode.isDark
                                              ? MaterialAlphaPrimitives.a100
                                              : MaterialAlphaPrimitives.a70
    readonly property real viewportContrastSupportAlpha: MaterialAlphaPrimitives.a16

    readonly property real transportPrimaryRestAlpha: ThemeMode.isDark
                                                      ? MaterialAlphaPrimitives.a100
                                                      : MaterialAlphaPrimitives.a48
    readonly property real transportPrimaryHoverAlpha: ThemeMode.isDark
                                                       ? MaterialAlphaPrimitives.a100
                                                       : MaterialAlphaPrimitives.a52
    readonly property real transportPrimaryPressedAlpha: ThemeMode.isDark
                                                         ? MaterialAlphaPrimitives.a70
                                                         : MaterialAlphaPrimitives.a42
}
