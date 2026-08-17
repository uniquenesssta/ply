import QtQuick
import Player.Presentation.Theme

Panel {
    surfaceColor: ColorTokens.surfaceInspectorDark
    fillAlpha: MaterialTokens.inspectorDarkFillAlpha
    borderColor: ColorTokens.borderInspectorDark
    borderAlpha: MaterialTokens.inspectorDarkBorderAlpha
    cornerRadius: RadiusTokens.surfaceInspector
    backdropBlurRadius: MaterialTokens.inspectorDarkBlur
    shadowColor: ElevationTokens.inspectorDarkShadowColor
    shadowRadius: ElevationTokens.inspectorDarkShadowRadius
    shadowXOffset: ElevationTokens.inspectorDarkShadowXOffset
    shadowYOffset: ElevationTokens.inspectorDarkShadowYOffset
    shadowAlpha: ElevationTokens.inspectorDarkShadowAlpha
    contentPadding: SpacingTokens.surfacePadding
    z: ZOrderTokens.inspector
}
