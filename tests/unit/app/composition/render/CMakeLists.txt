add_executable(
    player_video_render_binding_tests
    player_video_render_binding_test.cpp
    ../../../../../src/app/composition/render/player_video_render_binding.cpp
    ../../../../../src/app/composition/render/player_video_render_binding.h
)

player_apply_compiler_options(player_video_render_binding_tests)
player_enable_compiler_warnings(player_video_render_binding_tests)
player_enable_sanitizers(player_video_render_binding_tests)
player_enable_static_analysis(player_video_render_binding_tests)

target_include_directories(
    player_video_render_binding_tests
    PRIVATE
        ../../../../../src
)

target_link_libraries(
    player_video_render_binding_tests
    PRIVATE
        player_playback_application
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME player_video_render_binding
    COMMAND player_video_render_binding_tests
)
