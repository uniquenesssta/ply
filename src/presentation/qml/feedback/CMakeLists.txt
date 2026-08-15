set(PLAYER_FEEDBACK_QML_FILES
    StatusFeedbackBody.qml
    ActionFeedbackBody.qml
    Toast.qml
    ErrorFeedback.qml
    LoadingFeedback.qml
    BufferingFeedback.qml
    EndedFeedback.qml
    EmptyFeedback.qml
)

set_source_files_properties(
    StatusFeedbackBody.qml
    ActionFeedbackBody.qml
    PROPERTIES QT_QML_INTERNAL_TYPE TRUE
)

qt_add_qml_module(
    player_presentation_feedback
    STATIC
    URI Player.Presentation.Feedback
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    DEPENDENCIES
        QtQuick
        Player.Presentation.Theme
        Player.Presentation.Primitives
        Player.Presentation.Controls
        Player.Presentation.Surfaces
    QML_FILES
        ${PLAYER_FEEDBACK_QML_FILES}
)

target_link_libraries(
    player_presentation_feedback
    PRIVATE
        player_presentation_theme
        player_presentation_primitives
        player_presentation_controls
        player_presentation_surfaces
)

player_configure_presentation_qml_module(player_presentation_feedback)
