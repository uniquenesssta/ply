add_library(
    player_chapters_presentation
    STATIC
        chapter_model.cpp
        chapter_model.h
)

player_apply_compiler_options(player_chapters_presentation)
player_enable_compiler_warnings(player_chapters_presentation)
player_enable_sanitizers(player_chapters_presentation)
player_enable_static_analysis(player_chapters_presentation)

target_include_directories(
    player_chapters_presentation
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    player_chapters_presentation
    PUBLIC
        player_playback_domain
        Qt6::Core
)
