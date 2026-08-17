set(PLAYER_SURFACE_QML_FILES
    BackdropBlur.qml
    Panel.qml
    OscSurface.qml
    Drawer.qml
    Popover.qml
    Hud.qml
)

set_source_files_properties(
    BackdropBlur.qml
    PROPERTIES QT_QML_INTERNAL_TYPE TRUE
)

qt_add_qml_module(
    player_presentation_surfaces
    STATIC
    URI Player.Presentation.Surfaces
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    DEPENDENCIES
        QtQuick
        QtQuick.Effects
        Player.Presentation.Theme
    QML_FILES
        ${PLAYER_SURFACE_QML_FILES}
)

target_link_libraries(
    player_presentation_surfaces
    PRIVATE
        player_presentation_theme
)

player_configure_presentation_qml_module(player_presentation_surfaces)
