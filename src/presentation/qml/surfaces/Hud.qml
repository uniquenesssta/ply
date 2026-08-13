import QtQuick
import Player.Presentation.Theme

Panel {
    // Final Figma defines a HUD effect style but no independent HUD fill-alpha
    // semantic. Its effect tier matches Control Glass, so reuse that established
    // material fill instead of creating a second unsupported visual truth.
    fillAlpha: MaterialTokens.controlFillAlpha
    cornerRadius: RadiusTokens.surfaceHud
    backdropBlurRadius: MaterialTokens.hudBlur
    shadowRadius: ElevationTokens.controlShadowRadius
    shadowYOffset: ElevationTokens.controlShadowYOffset
    shadowAlpha: ElevationTokens.controlShadowAlpha
    contentPadding: SpacingTokens.surfacePaddingSmall
    z: ZOrderTokens.hud
}
