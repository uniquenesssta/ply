import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Panel {
    id: root

    property bool compact: false
    property bool windowExpanded: false

    signal minimizeRequested()
    signal maximizeRestoreRequested()
    signal closeRequested()

    objectName: "playerWindowActionsPod"
    implicitWidth: LayoutTokens.headerActionsWidth
    implicitHeight: root.compact
                    ? LayoutTokens.headerHeightCompact
                    : LayoutTokens.headerHeight
    cornerRadius: root.compact
                  ? RadiusTokens.headerCompact
                  : RadiusTokens.header
    fillAlpha: root.compact
               ? MaterialTokens.headerCompactFillAlpha
               : MaterialTokens.headerFillAlpha
    backdropBlurRadius: root.compact
                        ? MaterialTokens.headerCompactBlur
                        : MaterialTokens.headerBlur
    contentPadding: 0

    Row {
        anchors.centerIn: parent
        spacing: LayoutTokens.headerActionGap

        IconButton {
            objectName: "playerWindowMinimizeButton"
            iconId: "minimize"
            toolTipText: qsTr("Minimize")
            accessibleDescription: qsTr("Minimize the player window")
            onClicked: root.minimizeRequested()
        }

        IconButton {
            objectName: "playerWindowMaximizeRestoreButton"
            iconId: "maximize"
            toolTipText: root.windowExpanded ? qsTr("Restore") : qsTr("Maximize")
            accessibleDescription: root.windowExpanded
                                   ? qsTr("Restore the player window")
                                   : qsTr("Maximize the player window")
            onClicked: root.maximizeRestoreRequested()
        }

        IconButton {
            objectName: "playerWindowCloseButton"
            iconId: "close"
            toolTipText: qsTr("Close")
            accessibleDescription: qsTr("Close the player window")
            onClicked: root.closeRequested()
        }
    }
}
