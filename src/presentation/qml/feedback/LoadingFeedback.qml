import QtQuick
import Player.Presentation.Theme

StatusFeedbackBody {
    readonly property string feedbackRole: "loading"

    titleColor: ColorTokens.feedbackInfo
    z: ZOrderTokens.overlay
}
