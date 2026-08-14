set(PLAYER_SURFACE_QML_FILES
    Panel.qml
    OscSurface.qml
    Drawer.qml
    Popover.qml
    Hud.qml
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
