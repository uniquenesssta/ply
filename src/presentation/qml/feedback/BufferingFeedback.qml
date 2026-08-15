import QtQuick
import Player.Presentation.Theme

StatusFeedbackBody {
    readonly property string feedbackRole: "buffering"

    titleColor: ColorTokens.feedbackInfo
    z: ZOrderTokens.overlay
}
