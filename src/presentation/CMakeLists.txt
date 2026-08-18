set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")

add_subdirectory(qml)
add_subdirectory(viewmodels/player/transport)
add_subdirectory(viewmodels/player/timeline)
add_subdirectory(viewmodels/player/volume)
add_subdirectory(viewmodels/player/status)
add_subdirectory(viewmodels/player/media)
add_subdirectory(viewmodels/player/hud)

set(PLAYER_QML_FILES
    qml/App.qml
    qml/shell/MainWindow.qml
    qml/shell/theme/ThemeModeShortcut.qml
    qml/shell/window/FullscreenWindowController.qml
    qml/screens/player/PlayerScreen.qml
    qml/screens/player/layout/VideoViewport.qml
    qml/screens/player/layout/PlayerTopRegion.qml
    qml/screens/player/layout/PlayerBottomRegion.qml
    qml/screens/player/header/PlayerFloatingHeader.qml
    qml/screens/player/fullscreen/FullscreenHeader.qml
    qml/screens/player/osc/PlayerOscLayout.qml
    qml/screens/player/osc/OscControlRow.qml
    qml/screens/player/osc/PlayerUtilityControls.qml
    qml/screens/player/overlays/PlayerOverlayStack.qml
    qml/screens/player/overlays/status/PlayerStatusOverlay.qml
    qml/screens/player/overlays/hud/PlayerHudOverlay.qml
    qml/screens/player/overlays/drop/PlayerDropOverlay.qml
    qml/screens/player/drawers/PlayerDrawerHost.qml
    qml/screens/player/drawers/PlayerInspectorShell.qml
    qml/screens/player/drawers/PlaylistInspector.qml
    qml/screens/player/drawers/PlaylistSearchField.qml
    qml/screens/player/drawers/PlaylistFooter.qml
    qml/features/player/chrome/PlayerChromeVisibilityController.qml
    qml/features/player/chrome/PlayerCursorVisibilityController.qml
    qml/features/player/chrome/PlayerChromeActivityLayer.qml
    qml/features/player/video/VideoSurface.qml
    qml/features/player/transport/TransportControls.qml
    qml/features/player/timeline/TimelineControls.qml
    qml/features/player/volume/VolumeControls.qml
    qml/features/player/fullscreen/FullscreenControls.qml
    qml/features/player/fullscreen/FullscreenGestureLayer.qml
    qml/features/player/mediaopen/LocalMediaOpenDialog.qml
    qml/features/player/mediaopen/UrlMediaOpenDialog.qml
    qml/features/playlist/PlaylistControls.qml
    qml/features/playlist/PlaylistContent.qml
    qml/features/playlist/PlaylistRow.qml
    qml/features/tracks/TrackSelectionPopup.qml
)

target_sources(
    player_app
    PRIVATE
        qml/types/presentation_type_registration.cpp
        qml/types/presentation_type_registration.h
)

foreach(qml_file IN LISTS PLAYER_QML_FILES)
    get_filename_component(qml_name "${qml_file}" NAME)
    set_source_files_properties(
        "${qml_file}"
        PROPERTIES QT_RESOURCE_ALIAS "${qml_name}"
    )
endforeach()

set_source_files_properties(
    qml/shell/MainWindow.qml
    qml/shell/theme/ThemeModeShortcut.qml
    qml/shell/window/FullscreenWindowController.qml
    qml/screens/player/PlayerScreen.qml
    qml/screens/player/layout/VideoViewport.qml
    qml/screens/player/layout/PlayerTopRegion.qml
    qml/screens/player/layout/PlayerBottomRegion.qml
    qml/screens/player/header/PlayerFloatingHeader.qml
    qml/screens/player/fullscreen/FullscreenHeader.qml
    qml/screens/player/osc/PlayerOscLayout.qml
    qml/screens/player/osc/OscControlRow.qml
    qml/screens/player/osc/PlayerUtilityControls.qml
    qml/screens/player/overlays/PlayerOverlayStack.qml
    qml/screens/player/overlays/status/PlayerStatusOverlay.qml
    qml/screens/player/overlays/hud/PlayerHudOverlay.qml
    qml/screens/player/overlays/drop/PlayerDropOverlay.qml
    qml/screens/player/drawers/PlayerDrawerHost.qml
    qml/screens/player/drawers/PlayerInspectorShell.qml
    qml/screens/player/drawers/PlaylistInspector.qml
    qml/screens/player/drawers/PlaylistSearchField.qml
    qml/screens/player/drawers/PlaylistFooter.qml
    qml/features/player/chrome/PlayerChromeVisibilityController.qml
    qml/features/player/chrome/PlayerCursorVisibilityController.qml
    qml/features/player/chrome/PlayerChromeActivityLayer.qml
    qml/features/player/video/VideoSurface.qml
    qml/features/player/transport/TransportControls.qml
    qml/features/player/timeline/TimelineControls.qml
    qml/features/player/volume/VolumeControls.qml
    qml/features/player/fullscreen/FullscreenControls.qml
    qml/features/player/fullscreen/FullscreenGestureLayer.qml
    qml/features/player/mediaopen/LocalMediaOpenDialog.qml
    qml/features/player/mediaopen/UrlMediaOpenDialog.qml
    qml/features/playlist/PlaylistControls.qml
    qml/features/playlist/PlaylistContent.qml
    qml/features/playlist/PlaylistRow.qml
    qml/features/tracks/TrackSelectionPopup.qml
    PROPERTIES QT_QML_INTERNAL_TYPE TRUE
)

qt_add_qml_module(
    player_app
    URI Player.Presentation
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_GENERATE_QMLTYPES
    TYPEINFO player_app.qmltypes
    DEPENDENCIES
        QtQuick
        QtQuick.Controls
        QtQuick.Controls.Basic
        QtQuick.Dialogs
        Player.Presentation.Theme
        Player.Presentation.Primitives
        Player.Presentation.Controls
        Player.Presentation.Surfaces
        Player.Presentation.Feedback
    QML_FILES ${PLAYER_QML_FILES}
)

qt_query_qml_module(
    player_app
    TYPEINFO player_presentation_typeinfo_path
)

if(NOT player_presentation_typeinfo_path)
    message(FATAL_ERROR "Player.Presentation typeinfo output path was not resolved.")
endif()

get_filename_component(
    player_presentation_typeinfo_directory
    "${player_presentation_typeinfo_path}"
    DIRECTORY
)
file(MAKE_DIRECTORY "${player_presentation_typeinfo_directory}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/qml/types/player_presentation.qmltypes"
    "${player_presentation_typeinfo_path}"
    COPYONLY
)

target_link_libraries(
    player_app
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        player_presentation_controls
        player_presentation_surfaces
        player_presentation_feedback
        player_presentation_transport
        player_presentation_timeline
        player_presentation_volume
        player_presentation_status
        player_presentation_media
        player_presentation_hud
)

add_custom_target(
    player_qml_lint
    DEPENDS
        player_app_qmllint
        player_presentation_theme_qmllint
        player_presentation_primitives_qmllint
        player_presentation_controls_qmllint
        player_presentation_surfaces_qmllint
        player_presentation_feedback_qmllint
)
