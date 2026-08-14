import QtQuick
import Player.Presentation.Surfaces
import Player.Presentation.Theme

Item {
    id: root

    property bool compact: false

    property alias timelineContent: timelineHost.data
    property alias transportContent: controlRow.transportContent
    property alias volumeContent: controlRow.volumeContent
    property alias utilityContent: controlRow.utilityContent

    readonly property Item timelineItem: timelineHost
    readonly property Item transportItem: controlRow.transportItem
    readonly property Item volumeItem: controlRow.volumeItem
    readonly property Item utilityItem: controlRow.utilityItem
    readonly property real surfaceWidth: Math.min(
        LayoutTokens.oscMaximumWidth,
        Math.max(0, root.width))
    readonly property bool widthConstrained: root.surfaceWidth < LayoutTokens.oscMaximumWidth
                                             || controlRow.contentConstrained
    readonly property int horizontalInset: root.compact
                                            ? SpacingTokens.oscInsetCompact
                                            : SpacingTokens.oscInset
    readonly property int topInset: root.compact
                                    ? SpacingTokens.oscPaddingTopCompact
                                    : SpacingTokens.oscPaddingTop
    readonly property int laneGap: root.compact
                                   ? SpacingTokens.oscSectionGapCompact
                                   : SpacingTokens.oscSectionGap
    readonly property int timelineLaneHeight: root.compact
                                              ? LayoutTokens.oscTimelineLaneHeightCompact
                                              : LayoutTokens.oscTimelineLaneHeight

    objectName: "playerOscLayout"
    implicitHeight: root.compact
                    ? LayoutTokens.oscHeightCompact
                    : LayoutTokens.oscHeight

    OscSurface {
        id: surface

        objectName: "playerOscSurface"
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        width: root.surfaceWidth
        height: root.implicitHeight
        compact: root.compact

        Item {
            id: timelineHost

            objectName: "playerOscTimelineSlot"
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                leftMargin: root.horizontalInset
                rightMargin: root.horizontalInset
                topMargin: root.topInset
            }
            height: root.timelineLaneHeight
            clip: true
        }

        OscControlRow {
            id: controlRow

            anchors {
                left: parent.left
                right: parent.right
                top: timelineHost.bottom
                leftMargin: root.horizontalInset
                rightMargin: root.horizontalInset
                topMargin: root.laneGap
            }
            height: LayoutTokens.oscControlLaneHeight
            compact: root.compact
        }
    }
}
