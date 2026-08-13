import QtQuick
import Player.Presentation.Theme

Text {
    enum Variant {
        Meta,
        Technical,
        MicroStrong,
        CompactStrong
    }

    property int variant: CaptionText.Meta

    font: variant === CaptionText.Technical
          ? TypographyTokens.technicalMetadata
          : variant === CaptionText.MicroStrong
            ? TypographyTokens.microStrong
            : variant === CaptionText.CompactStrong
              ? TypographyTokens.compactStrong
              : TypographyTokens.metaBody
    color: ColorTokens.textMuted
    textFormat: Text.PlainText
    wrapMode: Text.NoWrap
    elide: Text.ElideRight
    maximumLineCount: 1
}
