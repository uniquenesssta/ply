set(PLAYER_CONTROL_QML_FILES
    buttons/ButtonBase.qml
    buttons/IconButton.qml
    buttons/TextButton.qml
    buttons/ToggleButton.qml
)

set_source_files_properties(
    buttons/ButtonBase.qml
    PROPERTIES QT_QML_INTERNAL_TYPE TRUE
)

qt_add_qml_module(
    player_presentation_controls
    STATIC
    URI Player.Presentation.Controls
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    DEPENDENCIES
        QtQuick
        Player.Presentation.Theme
        Player.Presentation.Primitives
    QML_FILES
        ${PLAYER_CONTROL_QML_FILES}
)

target_link_libraries(
    player_presentation_controls
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
)

player_configure_presentation_qml_module(player_presentation_controls)
