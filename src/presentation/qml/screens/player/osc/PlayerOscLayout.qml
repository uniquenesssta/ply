import QtQuick
import Player.Presentation.Surfaces
import Player.Presentation.Theme

FocusScope {
    id: root

    property bool compact: false
    property bool inspectorOpen: false

    property alias timelineContent: timelineHost.data
    property alias transportContent: controlRow.transportContent
    property alias volumeContent: controlRow.volumeContent
    property alias utilityContent: controlRow.utilityContent

    readonly property Item timelineItem: timelineHost
    readonly property Item transportItem: controlRow.transportItem
    readonly property Item volumeItem: controlRow.volumeItem
    readonly property Item utilityItem: controlRow.utilityItem
    readonly property bool controlsHovered: surfaceHover.hovered
    readonly property bool controlsFocused: root.activeFocus
    readonly property int maximumSurfaceWidth: root.compact
                                                ? LayoutTokens.oscMaximumWidthCompact
                                                : root.inspectorOpen
                                                  ? LayoutTokens.oscMaximumWidthInspector
                                                  : LayoutTokens.oscMaximumWidth
    readonly property real surfaceWidth: Math.min(
        root.maximumSurfaceWidth,
        Math.max(0, root.width))
    readonly property real requestedCenterOffset: !root.compact && root.inspectorOpen
                                                   ? -LayoutTokens.oscInspectorCenterOffset
                                                   : 0
    readonly property real maximumCenterOffset: Math.max(0, (root.width - root.surfaceWidth) / 2)
    readonly property real surfaceCenterOffset: Math.max(
        -root.maximumCenterOffset,
        Math.min(root.maximumCenterOffset, root.requestedCenterOffset))
    readonly property bool widthConstrained: root.surfaceWidth < root.maximumSurfaceWidth
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
            horizontalCenterOffset: root.surfaceCenterOffset
        }
        width: root.surfaceWidth
        height: root.implicitHeight
        compact: root.compact

        HoverHandler {
            id: surfaceHover
            objectName: "playerOscSurfaceHoverHandler"
        }

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
            clip: false
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
        }
    }
}
