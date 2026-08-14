import QtQuick
import Player.Presentation.Theme

Panel {
    id: root

    property bool compact: false

    objectName: "oscSurface"
    implicitWidth: LayoutTokens.oscMaximumWidth
    implicitHeight: root.compact
                    ? LayoutTokens.oscHeightCompact
                    : LayoutTokens.oscHeight
    cornerRadius: root.compact
                  ? RadiusTokens.surfaceOscCompact
                  : RadiusTokens.surfaceOsc
    fillAlpha: root.compact
               ? MaterialTokens.oscCompactFillAlpha
               : MaterialTokens.oscFillAlpha
    backdropBlurRadius: root.compact
                        ? MaterialTokens.oscCompactBlur
                        : MaterialTokens.oscBlur
    contentPadding: 0
    z: ZOrderTokens.osc
}
