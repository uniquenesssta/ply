pragma Singleton

import QtQuick

QtObject {
    readonly property color shadowColor: ColorPrimitives.shadowPlum

    readonly property int floatingShadowRadius: ShadowPrimitives.radius42
    readonly property int floatingShadowYOffset: ShadowPrimitives.y12
    readonly property int floatingShadowSpread: ShadowPrimitives.spread0
    readonly property real floatingShadowAlpha: MaterialAlphaPrimitives.a12

    readonly property int controlShadowRadius: ShadowPrimitives.radius16
    readonly property int controlShadowYOffset: ShadowPrimitives.y5
    readonly property int controlShadowSpread: ShadowPrimitives.spread0
    readonly property real controlShadowAlpha: MaterialAlphaPrimitives.a08

    readonly property int windowShadowRadius: ShadowPrimitives.radius70
    readonly property int windowShadowYOffset: ShadowPrimitives.y24
    readonly property int windowShadowSpread: ShadowPrimitives.spread0
    readonly property real windowShadowAlpha: MaterialAlphaPrimitives.a12

    readonly property int immersiveWindowShadowRadius: ShadowPrimitives.radius65
    readonly property int immersiveWindowShadowYOffset: ShadowPrimitives.y22
    readonly property int immersiveWindowShadowSpread: ShadowPrimitives.spread0
    readonly property real immersiveWindowShadowAlpha: MaterialAlphaPrimitives.a11
}
