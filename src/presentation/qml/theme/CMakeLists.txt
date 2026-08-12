set_source_files_properties(
    Theme.qml
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
        Theme.qml
)

player_configure_presentation_qml_module(player_presentation_theme)
