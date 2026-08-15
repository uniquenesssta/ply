add_library(
    player_media_domain
    STATIC
        media_source.cpp
        media_source.h
)

player_apply_compiler_options(player_media_domain)
player_enable_compiler_warnings(player_media_domain)
player_enable_sanitizers(player_media_domain)
player_enable_static_analysis(player_media_domain)

target_include_directories(
    player_media_domain
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_media_domain
    PUBLIC
        Qt6::Core
)
