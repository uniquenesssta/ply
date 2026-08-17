import QtQuick
import Player.Presentation.Theme

Panel {
    fillAlpha: MaterialTokens.inspectorFillAlpha
    borderAlpha: MaterialTokens.borderStrongAlpha
    cornerRadius: RadiusTokens.surfaceInspector
    backdropBlurRadius: MaterialTokens.inspectorBlur
    shadowRadius: ElevationTokens.floatingShadowRadius
    shadowYOffset: ElevationTokens.floatingShadowYOffset
    shadowAlpha: ElevationTokens.floatingShadowAlpha
    contentPadding: SpacingTokens.surfacePadding
    z: ZOrderTokens.inspector
}
