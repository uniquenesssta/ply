import QtQuick
import Player.Presentation.Theme

Text {
    enum Variant {
        MediumPrimary,
        MediumSecondary,
        SmallPrimary,
        SmallSecondary,
        ExtraSmall
    }

    property int variant: TimecodeText.MediumPrimary

    font: variant === TimecodeText.MediumSecondary
          ? TypographyTokens.timecodeMediumSecondary
          : variant === TimecodeText.SmallPrimary
            ? TypographyTokens.timecodeSmallPrimary
            : variant === TimecodeText.SmallSecondary
              ? TypographyTokens.timecodeSmallSecondary
              : variant === TimecodeText.ExtraSmall
                ? TypographyTokens.timecodeExtraSmall
                : TypographyTokens.timecodeMediumPrimary
    color: variant === TimecodeText.MediumSecondary
           || variant === TimecodeText.SmallSecondary
           || variant === TimecodeText.ExtraSmall
           ? ColorTokens.textSecondary
           : ColorTokens.textPrimary
    textFormat: Text.PlainText
    wrapMode: Text.NoWrap
    elide: Text.ElideNone
    maximumLineCount: 1
}
