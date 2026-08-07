qt_add_executable(player_app MANUAL_FINALIZATION)

add_subdirectory(foundation)
add_subdirectory(app)
add_subdirectory(presentation)

player_configure_application_target(player_app)

install(
    TARGETS player_app
    RUNTIME DESTINATION .
    BUNDLE DESTINATION .
)

qt_finalize_executable(player_app)
