set(PLAYER_QML_FILES
    qml/App.qml
    qml/shell/MainWindow.qml
    qml/screens/player/PlayerScreen.qml
    qml/features/player/video/VideoSurface.qml
    qml/features/player/chrome/PlayerChrome.qml
    qml/theme/Theme.qml
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
    # The scaffold currently exposes no C++ types to QML. Avoid generating an
    # empty metatypes document until the first Q_OBJECT/QML_ELEMENT type is added.
    NO_GENERATE_QMLTYPES
    QML_FILES ${PLAYER_QML_FILES}
)
