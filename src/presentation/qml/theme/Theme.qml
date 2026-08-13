pragma Singleton

import QtQuick

QtObject {
    // Compatibility facade kept for R5-01 consumers. New UI code should prefer
    // the responsibility-specific semantic token singletons directly.
    readonly property color windowBackground: ColorTokens.surfaceCanvas
    readonly property color videoBackground: ColorTokens.surfaceVideo
    readonly property color chromeBackground: ColorTokens.surfaceGlass
    readonly property color primaryText: ColorTokens.textPrimary
    readonly property color secondaryText: ColorTokens.textSecondary
}
