import QtQuick
import QtQuick.Controls.Basic
import Player.Presentation.Theme
import Player.Presentation.Controls

// Chapter list menu: clicking a chapter issues one absolute Seek intent
// through the TimelineViewModel. Markers stay a read-only projection; this
// menu never writes Timeline state directly.
Popup {
    id: root

    property var chapterListModel: null
    property var timelineViewModel: null

    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: SpacingTokens.surfacePaddingSmall

    width: Math.min(
        320,
        parent !== null ? parent.width : 320)
    implicitHeight: contentColumn.implicitHeight + (SpacingTokens.surfacePaddingSmall * 2)

    background: Rectangle {
        radius: RadiusTokens.surfacePopover
        color: Qt.rgba(ColorTokens.surfaceGlassStrong.r,
                       ColorTokens.surfaceGlassStrong.g,
                       ColorTokens.surfaceGlassStrong.b,
                       MaterialTokens.popoverFillAlpha)
        border.width: LayoutTokens.surfaceBorderWidth
        border.color: Qt.rgba(ColorTokens.borderGlass.r,
                              ColorTokens.borderGlass.g,
                              ColorTokens.borderGlass.b,
                              MaterialTokens.borderStrongAlpha)
    }

    contentItem: Column {
        id: contentColumn

        spacing: SpacingTokens.controlAdjacent

        CaptionText {
            width: parent.width
            text: qsTr("Chapters")
            horizontalAlignment: Text.AlignLeft
        }

        ListView {
            id: chapterList

            objectName: "chapterList"
            width: parent.width
            height: Math.min(
                contentHeight,
                LayoutTokens.windowMinimumHeight * 0.4)
            clip: true
            interactive: contentHeight > height
            model: root.chapterListModel

            delegate: TextButton {
                objectName: "chapterRow"
                width: chapterList.width
                text: {
                    const timecode = root.formatTimecode(model.startSeconds)
                    return model.title.length > 0
                           ? model.title + "  ·  " + timecode
                           : timecode
                }
                textColor: ColorTokens.textPrimary
                enabled: root.timelineViewModel !== null
                         && root.timelineViewModel.canSeek

                onClicked: {
                    if (root.timelineViewModel !== null) {
                        root.timelineViewModel.requestAbsoluteSeek(model.startSeconds)
                        root.close()
                    }
                }
            }
        }
    }

    function formatTimecode(secondsValue) {
        const total = Math.max(0, Math.floor(secondsValue))
        const hours = Math.floor(total / 3600)
        const minutes = Math.floor((total % 3600) / 60)
        const seconds = total % 60
        return (hours > 0
                ? (hours < 10 ? "0" + hours : hours) + ":"
                : "") +
               (minutes < 10 ? "0" + minutes : minutes) + ":" +
               (seconds < 10 ? "0" + seconds : seconds)
    }
}
