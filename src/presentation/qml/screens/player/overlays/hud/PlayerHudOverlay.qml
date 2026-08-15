pragma ComponentBehavior: Bound

import QtQuick
import Player.Presentation.Primitives
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Item {
    id: root

    property var viewModel: null
    property bool suppressed: false

    readonly property bool hudVisible: root.viewModel !== null
                                       && root.viewModel.visible
                                       && !root.suppressed
    readonly property string messageKey: root.viewModel !== null
                                         ? root.viewModel.messageKey
                                         : ""
    readonly property string valueText: root.viewModel !== null
                                        ? root.viewModel.valueText
                                        : ""
    readonly property string labelText: {
        switch (root.messageKey) {
        case "volume":
            return qsTr("Volume")
        case "muted":
            return qsTr("Muted")
        case "seek":
            return qsTr("Seek")
        case "seekFailed":
            return qsTr("Seek failed")
        case "speed":
            return qsTr("Speed")
        case "track":
            return qsTr("Track")
        default:
            return ""
        }
    }
    readonly property string primaryText: root.messageKey === "muted"
                                          || root.messageKey === "seekFailed"
                                          ? root.labelText
                                          : root.valueText

    objectName: "playerHudOverlay"
    enabled: false
    z: ZOrderTokens.hud
    opacity: root.hudVisible ? OpacityTokens.visible : OpacityTokens.hidden
    visible: root.hudVisible || root.opacity > OpacityTokens.hidden

    Behavior on opacity {
        NumberAnimation {
            duration: root.hudVisible
                      ? MotionTokens.hudShowDuration
                      : MotionTokens.hudHideDuration
            easing.type: root.hudVisible
                         ? MotionTokens.enterEasingType
                         : MotionTokens.exitEasingType
            easing.bezierCurve: root.hudVisible
                                ? MotionTokens.enterBezier
                                : MotionTokens.exitBezier
        }
    }

    Hud {
        id: hudSurface
        objectName: "playerHudSurface"
        anchors.centerIn: parent
        width: hudBody.implicitWidth + contentPadding * 2
        height: hudBody.implicitHeight + contentPadding * 2

        Column {
            id: hudBody
            anchors.centerIn: parent
            spacing: SpacingTokens.controlTight

            CaptionText {
                id: hudLabel
                objectName: "playerHudLabel"
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.messageKey !== "muted"
                         && root.messageKey !== "seekFailed"
                         && text.length > 0
                variant: CaptionText.CompactStrong
                text: root.labelText
            }

            TitleText {
                id: hudValue
                objectName: "playerHudPrimaryText"
                anchors.horizontalCenter: parent.horizontalCenter
                variant: TitleText.MediaCompact
                text: root.primaryText
            }
        }
    }
}
