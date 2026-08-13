set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")

add_subdirectory(qml)

set(PLAYER_QML_FILES
    qml/App.qml
    qml/shell/MainWindow.qml
    qml/screens/player/PlayerScreen.qml
    qml/features/player/video/VideoSurface.qml
    qml/features/player/chrome/PlayerChrome.qml
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
    qml/screens/player/PlayerScreen.qml
    qml/features/player/video/VideoSurface.qml
    qml/features/player/chrome/PlayerChrome.qml
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
        Player.Presentation.Theme
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
)

add_custom_target(
    player_qml_lint
    DEPENDS
        player_app_qmllint
        player_presentation_theme_qmllint
        player_presentation_primitives_qmllint
        player_presentation_controls_qmllint
)
