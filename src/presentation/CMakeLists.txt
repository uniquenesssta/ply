set(PLAYER_QML_FILES
    qml/App.qml
    qml/shell/MainWindow.qml
    qml/screens/player/PlayerScreen.qml
    qml/features/player/video/VideoSurface.qml
    qml/features/player/chrome/PlayerChrome.qml
    qml/theme/Theme.qml
)

set(PLAYER_QML_CPP_SOURCES
    qml/types/mpv_video_item_qml.h
)

foreach(qml_file IN LISTS PLAYER_QML_FILES)
    get_filename_component(qml_name "${qml_file}" NAME)
    set_source_files_properties(
        "${qml_file}"
        PROPERTIES QT_RESOURCE_ALIAS "${qml_name}"
    )
endforeach()

set_source_files_properties(
    qml/theme/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(
    player_app
    URI Player.Presentation
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES ${PLAYER_QML_CPP_SOURCES}
    QML_FILES ${PLAYER_QML_FILES}
)
