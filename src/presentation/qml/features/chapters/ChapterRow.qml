import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Theme

IconButton {
    id: root

    required property var chapterIndex
    required property string title
    required property string timeText
    property bool current: false
    property bool pending: false
    property bool seekEnabled: false

    signal seekRequested(var chapterIndex)

    objectName: "chapterRow"
    height: LayoutTokens.listRowHeight
    enabled: root.seekEnabled
    accessibleName: root.title
    accessibleDescription: root.timeText

    function withAlpha(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b,
                       colorValue.a * alphaValue)
    }

    function chapterNumberText() {
        const number = Number(root.chapterIndex) + 1
        return number < 10 ? "0" + String(number) : String(number)
    }

    Rectangle {
        anchors.fill: parent
        radius: RadiusTokens.listRow
        color: root.current
               ? root.withAlpha(ColorTokens.surfaceInspectorSelection,
                                MaterialTokens.selectionFillAlpha)
               : root.hovered
                 ? root.withAlpha(ColorTokens.surfaceInspectorSearch,
                                  MaterialTokens.footerFillAlpha)
                 : root.withAlpha(ColorTokens.surfaceInspectorRow,
                                  MaterialTokens.rowFillAlpha)
        border.width: root.current || root.pending || root.activeFocus
                      ? LayoutTokens.surfaceBorderWidth
                      : 0
        border.color: root.activeFocus
                      ? root.withAlpha(ColorTokens.focusRing,
                                       OpacityTokens.focusRing)
                      : root.pending
                        ? root.withAlpha(ColorTokens.controlPendingTarget,
                                         MaterialTokens.selectionBorderAlpha)
                        : root.withAlpha(ColorTokens.borderSelection,
                                         MaterialTokens.selectionBorderAlpha)
    }

    Rectangle {
        id: indexBadge

        anchors {
            left: parent.left
            leftMargin: SpacingTokens.listRowIndex
            verticalCenter: parent.verticalCenter
        }
        width: LayoutTokens.controlHitMinimum
        height: width
        radius: RadiusTokens.chapterIndexBadge
        color: root.withAlpha(root.current
                              ? ColorTokens.surfaceInspectorSelection
                              : ColorTokens.surfaceGlassSubtle,
                              MaterialTokens.footerFillAlpha)
        border.width: LayoutTokens.surfaceBorderWidth
        border.color: root.withAlpha(ColorTokens.borderGlass,
                                     MaterialTokens.borderSoftAlpha)

        Text {
            anchors.centerIn: parent
            text: root.chapterNumberText()
            color: root.current
                   ? ColorTokens.accentStrong
                   : ColorTokens.textMuted
            font: TypographyTokens.microStrong
        }
    }

    Item {
        anchors {
            left: indexBadge.right
            right: action.left
            top: parent.top
            bottom: parent.bottom
            leftMargin: SpacingTokens.controlTight
            rightMargin: SpacingTokens.controlTight
        }

        Text {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: SpacingTokens.listRowTitleTop
            }
            text: root.title
            color: ColorTokens.textStrong
            font: TypographyTokens.mediaTitleCompact
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }

        Text {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: SpacingTokens.listRowMetaTop
            }
            text: root.current
                  ? root.timeText + " · " + qsTr("Current Chapter")
                  : root.pending
                    ? root.timeText + " · " + qsTr("Seeking")
                    : root.timeText
            color: root.pending
                   ? ColorTokens.controlPendingTarget
                   : root.current
                     ? ColorTokens.accentStrong
                     : ColorTokens.textMuted
            font: TypographyTokens.timecodeExtraSmall
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }
    }

    Item {
        id: action

        anchors {
            right: parent.right
            rightMargin: SpacingTokens.listRowIndex
            verticalCenter: parent.verticalCenter
        }
        width: LayoutTokens.chapterRowActionSize
        height: LayoutTokens.chapterRowActionSize

        ChapterMarkerGlyph {
            anchors.centerIn: parent
            width: LayoutTokens.chapterRowActionGlyphSize
            height: LayoutTokens.chapterRowActionGlyphSize
            color: root.pending
                   ? ColorTokens.controlPendingTarget
                   : ColorTokens.selectionIndicator
            visible: root.current || root.pending
        }

        Text {
            anchors.centerIn: parent
            visible: !root.current && !root.pending
            text: "›"
            color: ColorTokens.textMuted
            font: TypographyTokens.controlBody
        }
    }

    onClicked: root.seekRequested(root.chapterIndex)
}
