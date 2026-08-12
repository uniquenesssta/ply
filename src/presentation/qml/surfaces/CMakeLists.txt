qt_add_qml_module(
    player_presentation_surfaces
    STATIC
    URI Player.Presentation.Surfaces
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
)

player_configure_presentation_qml_module(player_presentation_surfaces)
