set(PLAYER_PRIMITIVE_QML_FILES
    Icon.qml
    IconCatalog.qml
    text/TitleText.qml
    text/BodyText.qml
    text/CaptionText.qml
    text/TimecodeText.qml
)

set(PLAYER_ICON_ASSETS
    ../assets/icons/close.svg
    ../assets/icons/fullscreen.svg
    ../assets/icons/maximize.svg
    ../assets/icons/minimize.svg
    ../assets/icons/next.svg
    ../assets/icons/play.svg
    ../assets/icons/playlist.svg
    ../assets/icons/previous.svg
    ../assets/icons/search.svg
    ../assets/icons/subtitles.svg
    ../assets/icons/volume.svg
)

set_source_files_properties(
    IconCatalog.qml
    PROPERTIES QT_QML_INTERNAL_TYPE TRUE
)

foreach(icon_file IN LISTS PLAYER_ICON_ASSETS)
    get_filename_component(icon_name "${icon_file}" NAME)
    set_source_files_properties(
        "${icon_file}"
        PROPERTIES QT_RESOURCE_ALIAS "assets/icons/${icon_name}"
    )
endforeach()

qt_add_qml_module(
    player_presentation_primitives
    STATIC
    URI Player.Presentation.Primitives
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    DEPENDENCIES
        QtQuick
        QtQuick.Effects
        Player.Presentation.Theme
    QML_FILES
        ${PLAYER_PRIMITIVE_QML_FILES}
    RESOURCES
        ${PLAYER_ICON_ASSETS}
)

target_link_libraries(
    player_presentation_primitives
    PRIVATE
        player_presentation_theme
)

player_configure_presentation_qml_module(player_presentation_primitives)
