import QtQuick
import Player.Presentation.Theme

Panel {
    fillAlpha: MaterialTokens.popoverFillAlpha
    cornerRadius: RadiusTokens.surfacePopover
    backdropBlurRadius: MaterialTokens.popoverBlur
    shadowRadius: ElevationTokens.controlShadowRadius
    shadowYOffset: ElevationTokens.controlShadowYOffset
    shadowAlpha: ElevationTokens.controlShadowAlpha
    contentPadding: SpacingTokens.surfacePaddingSmall
    z: ZOrderTokens.popover
}
