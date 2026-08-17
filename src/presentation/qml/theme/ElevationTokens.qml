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

    readonly property color inspectorDarkShadowColor: ColorPrimitives.shadowNight
    readonly property int inspectorDarkShadowRadius: ShadowPrimitives.radius48
    readonly property int inspectorDarkShadowXOffset: ShadowPrimitives.xMinus6
    readonly property int inspectorDarkShadowYOffset: ShadowPrimitives.y18
    readonly property int inspectorDarkShadowSpread: ShadowPrimitives.spread0
    readonly property real inspectorDarkShadowAlpha: MaterialAlphaPrimitives.a48

    readonly property color inspectorDarkFieldShadowColor: ColorPrimitives.shadowNightSoft
    readonly property int inspectorDarkFieldShadowRadius: ShadowPrimitives.radius24
    readonly property int inspectorDarkFieldShadowYOffset: ShadowPrimitives.y8
    readonly property int inspectorDarkFieldShadowSpread: ShadowPrimitives.spread0
    readonly property real inspectorDarkFieldShadowAlpha: MaterialAlphaPrimitives.a26

    readonly property color inspectorDarkFooterShadowColor: ColorPrimitives.shadowNightSoft
    readonly property int inspectorDarkFooterShadowRadius: ShadowPrimitives.radius28
    readonly property int inspectorDarkFooterShadowYOffset: ShadowPrimitives.y10
    readonly property int inspectorDarkFooterShadowSpread: ShadowPrimitives.spread0
    readonly property real inspectorDarkFooterShadowAlpha: MaterialAlphaPrimitives.a28

    readonly property int windowShadowRadius: ShadowPrimitives.radius70
    readonly property int windowShadowYOffset: ShadowPrimitives.y24
    readonly property int windowShadowSpread: ShadowPrimitives.spread0
    readonly property real windowShadowAlpha: MaterialAlphaPrimitives.a12

    readonly property int immersiveWindowShadowRadius: ShadowPrimitives.radius65
    readonly property int immersiveWindowShadowYOffset: ShadowPrimitives.y22
    readonly property int immersiveWindowShadowSpread: ShadowPrimitives.spread0
    readonly property real immersiveWindowShadowAlpha: MaterialAlphaPrimitives.a11
}
