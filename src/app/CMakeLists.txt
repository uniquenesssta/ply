target_sources(
    player_app
    PRIVATE
        main.cpp
        bootstrap/application_bootstrap.cpp
        bootstrap/application_bootstrap.h
        bootstrap/graphics_backend_bootstrap.cpp
        bootstrap/graphics_backend_bootstrap.h
        bootstrap/logging_bootstrap.cpp
        bootstrap/logging_bootstrap.h
        bootstrap/qml_bootstrap.cpp
        bootstrap/qml_bootstrap.h
        bootstrap/runtime_paths.cpp
        bootstrap/runtime_paths.h
)

target_include_directories(
    player_app
    PRIVATE
        ..
)

target_link_libraries(
    player_app
    PRIVATE
        player_foundation
)
