import QtQuick
import Player.Presentation.Theme

Text {
    enum Variant {
        Inspector,
        Media,
        MediaCompact
    }

    property int variant: TitleText.Media

    font: variant === TitleText.Inspector
          ? TypographyTokens.inspectorTitle
          : variant === TitleText.MediaCompact
            ? TypographyTokens.mediaTitleCompact
            : TypographyTokens.mediaTitle
    color: ColorTokens.textPrimary
    textFormat: Text.PlainText
    wrapMode: Text.NoWrap
    elide: Text.ElideRight
    maximumLineCount: 1
}
