import QtQuick
import Player.Presentation.Theme

ActionFeedbackBody {
    readonly property string feedbackRole: "error"
    titleColor: ColorTokens.feedbackError
    z: ZOrderTokens.overlay
}
