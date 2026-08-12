qt_add_qml_module(
    player_presentation_controls
    STATIC
    URI Player.Presentation.Controls
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    NO_GENERATE_QMLTYPES
)

player_configure_presentation_qml_module(player_presentation_controls)
