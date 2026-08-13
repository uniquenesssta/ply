import QtQuick
import Player.Presentation.Theme
import Player.Presentation.Surfaces

Panel {
    id: root

    enum Tone {
        Neutral,
        Info,
        Warning,
        Error
    }

    property string title: ""
    property string message: ""
    property int tone: Toast.Neutral
    property real textWidth: 0

    readonly property string feedbackRole: "toast"
    readonly property color toneColor: tone === Toast.Info
                                      ? ColorTokens.feedbackInfo
                                      : tone === Toast.Warning
                                        ? ColorTokens.feedbackWarning
                                        : tone === Toast.Error
                                          ? ColorTokens.feedbackError
                                          : ColorTokens.feedbackNeutral

    cornerRadius: RadiusTokens.surfaceToast
    fillAlpha: MaterialTokens.popoverFillAlpha
    backdropBlurRadius: MaterialTokens.popoverBlur
    shadowRadius: ElevationTokens.controlShadowRadius
    shadowYOffset: ElevationTokens.controlShadowYOffset
    shadowAlpha: ElevationTokens.controlShadowAlpha
    contentPadding: SpacingTokens.surfacePaddingSmall
    z: ZOrderTokens.toast

    implicitWidth: messageBody.implicitWidth + (contentPadding * 2)
    implicitHeight: messageBody.implicitHeight + (contentPadding * 2)
    width: implicitWidth
    height: implicitHeight

    StatusFeedbackBody {
        id: messageBody
        title: root.title
        detail: root.message
        titleColor: root.toneColor
        textWidth: root.textWidth
    }
}
