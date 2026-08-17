set(PLAYER_THEME_SINGLETONS
    Theme.qml
    ThemeMode.qml
    ColorPrimitives.qml
    ColorTokens.qml
    TypographyPrimitives.qml
    TypographyTokens.qml
    SpacingPrimitives.qml
    SpacingTokens.qml
    SizePrimitives.qml
    LayoutTokens.qml
    RadiusPrimitives.qml
    RadiusTokens.qml
    BlurPrimitives.qml
    ShadowPrimitives.qml
    MaterialAlphaPrimitives.qml
    MaterialTokens.qml
    ElevationTokens.qml
    MotionPrimitives.qml
    MotionTokens.qml
    OpacityPrimitives.qml
    OpacityTokens.qml
    ZOrderPrimitives.qml
    ZOrderTokens.qml
)

set_source_files_properties(
    ${PLAYER_THEME_SINGLETONS}
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(
    player_presentation_theme
    STATIC
    URI Player.Presentation.Theme
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    QML_FILES
        ${PLAYER_THEME_SINGLETONS}
)

player_configure_presentation_qml_module(player_presentation_theme)