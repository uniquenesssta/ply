target_sources(
    player_app
    PRIVATE
        main.cpp
        bootstrap/application_bootstrap.cpp
        bootstrap/application_bootstrap.h
        bootstrap/graphics_backend_bootstrap.cpp
        bootstrap/graphics_backend_bootstrap.h
        bootstrap/qml_bootstrap.cpp
        bootstrap/qml_bootstrap.h
)

target_include_directories(
    player_app
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)
