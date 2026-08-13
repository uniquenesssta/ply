import QtQuick
import Player.Presentation.Theme

Text {
    enum Variant {
        Supporting,
        Control
    }

    property int variant: BodyText.Control

    font: variant === BodyText.Supporting
          ? TypographyTokens.supportingBody
          : TypographyTokens.controlBody
    color: ColorTokens.textSecondary
    textFormat: Text.PlainText
    wrapMode: Text.WordWrap
}
