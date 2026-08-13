import QtQuick
import Player.Presentation.Controls

StatusFeedbackBody {
    id: root

    property string actionText: ""

    signal actionRequested()

    TextButton {
        id: actionButton
        objectName: "feedbackAction"
        text: root.actionText
        visible: text.length > 0
        onClicked: root.actionRequested()
    }
}
