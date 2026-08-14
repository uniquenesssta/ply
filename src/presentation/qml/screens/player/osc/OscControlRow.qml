import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    property bool compact: false

    property alias transportContent: transportHost.data
    property alias volumeContent: volumeHost.data
    property alias utilityContent: utilityHost.data

    readonly property Item transportItem: transportHost
    readonly property Item volumeItem: volumeHost
    readonly property Item utilityItem: utilityHost
    readonly property int groupGap: root.compact
                                    ? SpacingTokens.controlAdjacent
                                    : SpacingTokens.controlGroup
    readonly property bool contentConstrained: leadingRow.width > leadingClip.width
                                                || utilityHost.implicitWidth > root.width

    objectName: "playerOscControlRow"
    clip: true

    Row {
        id: utilityHost

        objectName: "playerOscUtilitySlot"
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        width: Math.min(implicitWidth, root.width)
        height: root.height
        clip: true
    }

    Item {
        id: leadingClip

        objectName: "playerOscLeadingClip"
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            right: utilityHost.left
            rightMargin: utilityHost.width > 0 ? root.groupGap : 0
        }
        clip: true

        Row {
            id: leadingRow

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            width: implicitWidth
            height: root.height
            spacing: transportHost.width > 0 && volumeHost.width > 0
                     ? root.groupGap
                     : 0

            Row {
                id: transportHost

                objectName: "playerOscTransportSlot"
                width: implicitWidth
                height: root.height
            }

            Row {
                id: volumeHost

                objectName: "playerOscVolumeSlot"
                width: implicitWidth
                height: root.height
            }
        }
    }
}
