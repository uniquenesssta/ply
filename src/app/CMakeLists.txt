target_sources(
    player_app
    PRIVATE
        main.cpp
        bootstrap/application_bootstrap.cpp
        bootstrap/application_bootstrap.h
        bootstrap/logging_bootstrap.cpp
        bootstrap/logging_bootstrap.h
        bootstrap/qml_bootstrap.cpp
        bootstrap/qml_bootstrap.h
        bootstrap/runtime_paths.cpp
        bootstrap/runtime_paths.h
)

add_subdirectory(bootstrap/graphics_backend)
add_subdirectory(composition)

target_include_directories(
    player_app
    PRIVATE
        ..
)

target_link_libraries(
    player_app
    PRIVATE
        player_foundation
        player_mpv_infrastructure
        player_playback_application
)
