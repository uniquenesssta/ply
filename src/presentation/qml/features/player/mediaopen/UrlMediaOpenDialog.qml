import QtQuick
import QtQuick.Controls
import Player.Presentation.Theme
import Player.Presentation.Primitives
import Player.Presentation.Controls
import Player.Presentation.Surfaces

Popup {
    id: root

    property string errorKey: ""
    signal urlSubmitted(string sourceText)

    modal: true
    focus: true
    width: Math.min(520,
                    parent !== null
                    ? Math.max(320, parent.width - (SpacingTokens.windowSafeMinimum * 2))
                    : 520)
    height: contentColumn.implicitHeight + (SpacingTokens.surfacePaddingLarge * 2)
    padding: SpacingTokens.surfacePaddingLarge
    x: parent !== null ? Math.round((parent.width - width) / 2) : 0
    y: parent !== null ? Math.round((parent.height - height) / 2) : 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    function errorText(key) {
        switch (key) {
        case "empty-url":
            return qsTr("Enter a media URL")
        case "invalid-url":
            return qsTr("Enter a valid absolute URL")
        case "unsupported-url-scheme":
            return qsTr("Only HTTP and HTTPS URLs are supported")
        case "submission-rejected":
            return qsTr("The player is not ready to open this URL")
        case "workflow-unavailable":
            return qsTr("URL opening is unavailable")
        default:
            return ""
        }
    }

    function submit() {
        root.errorKey = ""
        root.urlSubmitted(sourceField.text)
    }

    onOpened: sourceField.forceActiveFocus()
    onClosed: {
        sourceField.text = ""
        root.errorKey = ""
    }

    background: Panel {
        cornerRadius: RadiusTokens.surfaceDialog
        fillAlpha: MaterialTokens.popoverFillAlpha
        backdropBlurRadius: MaterialTokens.dialogBlur
        contentPadding: 0
    }

    Overlay.modal: Rectangle {
        color: Qt.rgba(ColorTokens.surfaceLetterbox.r,
                       ColorTokens.surfaceLetterbox.g,
                       ColorTokens.surfaceLetterbox.b,
                       0.16)
    }

    contentItem: Column {
        id: contentColumn

        spacing: SpacingTokens.controlAdjacent

        TitleText {
            width: parent.width
            text: qsTr("Open URL")
            variant: TitleText.Media
            color: ColorTokens.textPrimary
        }

        BodyText {
            width: parent.width
            text: qsTr("Enter a direct HTTP or HTTPS media URL")
            variant: BodyText.Supporting
            color: ColorTokens.textSecondary
            wrapMode: Text.WordWrap
        }

        TextField {
            id: sourceField

            width: parent.width
            height: LayoutTokens.inspectorSearchHeight
            placeholderText: qsTr("https://example.com/video.mp4")
            color: ColorTokens.textPrimary
            placeholderTextColor: ColorTokens.textMuted
            selectionColor: ColorTokens.selectionBackground
            selectedTextColor: ColorTokens.textPrimary
            font: TypographyTokens.supportingBody
            selectByMouse: true
            inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase

            onTextChanged: root.errorKey = ""
            onAccepted: root.submit()

            background: Rectangle {
                radius: RadiusTokens.controlSearch
                color: Qt.rgba(ColorTokens.surfaceGlass.r,
                               ColorTokens.surfaceGlass.g,
                               ColorTokens.surfaceGlass.b,
                               MaterialTokens.fieldFillAlpha)
                border.width: LayoutTokens.surfaceBorderWidth
                border.color: sourceField.activeFocus
                              ? ColorTokens.borderFocus
                              : Qt.rgba(ColorTokens.borderGlass.r,
                                        ColorTokens.borderGlass.g,
                                        ColorTokens.borderGlass.b,
                                        MaterialTokens.borderSoftAlpha)
            }
        }

        BodyText {
            width: parent.width
            text: root.errorText(root.errorKey)
            variant: BodyText.Supporting
            color: ColorTokens.feedbackError
            visible: text.length > 0
            wrapMode: Text.WordWrap
        }

        Row {
            anchors.right: parent.right
            spacing: SpacingTokens.controlAdjacent

            TextButton {
                text: qsTr("Cancel")
                onClicked: root.close()
            }

            TextButton {
                text: qsTr("Open")
                enabled: sourceField.text.trim().length > 0
                onClicked: root.submit()
            }
        }
    }
}
