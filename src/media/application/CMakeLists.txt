add_library(
    player_media_application
    STATIC
        drop/media_drop_handler.cpp
        drop/media_drop_handler.h
        open/local_media_validator.cpp
        open/local_media_validator.h
        open/media_open_coordinator.cpp
        open/media_open_coordinator.h
        open/media_open_error.cpp
        open/media_open_error.h
)

player_apply_compiler_options(player_media_application)
player_enable_compiler_warnings(player_media_application)
player_enable_sanitizers(player_media_application)
player_enable_static_analysis(player_media_application)

target_include_directories(
    player_media_application
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_media_application
    PUBLIC
        player_media_domain
        Qt6::Core
)
