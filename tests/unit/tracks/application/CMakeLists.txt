add_executable(
    track_selection_controller_tests
    track_selection_controller_test.cpp
)

player_apply_compiler_options(track_selection_controller_tests)
player_enable_compiler_warnings(track_selection_controller_tests)
player_enable_sanitizers(track_selection_controller_tests)
player_enable_static_analysis(track_selection_controller_tests)

target_link_libraries(
    track_selection_controller_tests
    PRIVATE
        player_tracks_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME track_selection_controller
    COMMAND track_selection_controller_tests
)

add_executable(
    external_subtitle_flow_tests
    external_subtitle_flow_test.cpp
)

player_apply_compiler_options(external_subtitle_flow_tests)
player_enable_compiler_warnings(external_subtitle_flow_tests)
player_enable_sanitizers(external_subtitle_flow_tests)
player_enable_static_analysis(external_subtitle_flow_tests)

target_compile_definitions(
    external_subtitle_flow_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

target_link_libraries(
    external_subtitle_flow_tests
    PRIVATE
        player_tracks_application
        player_playback_application
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME external_subtitle_flow
    COMMAND external_subtitle_flow_tests
)

add_executable(
    subtitle_delay_flow_tests
    subtitle_delay_flow_test.cpp
)

player_apply_compiler_options(subtitle_delay_flow_tests)
player_enable_compiler_warnings(subtitle_delay_flow_tests)
player_enable_sanitizers(subtitle_delay_flow_tests)
player_enable_static_analysis(subtitle_delay_flow_tests)

target_compile_definitions(
    subtitle_delay_flow_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

target_link_libraries(
    subtitle_delay_flow_tests
    PRIVATE
        player_tracks_application
        player_playback_application
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME subtitle_delay_flow
    COMMAND subtitle_delay_flow_tests
)

add_executable(
    audio_delay_controller_tests
    audio_delay_controller_test.cpp
)

player_apply_compiler_options(audio_delay_controller_tests)
player_enable_compiler_warnings(audio_delay_controller_tests)
player_enable_sanitizers(audio_delay_controller_tests)
player_enable_static_analysis(audio_delay_controller_tests)

target_link_libraries(
    audio_delay_controller_tests
    PRIVATE
        player_tracks_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME audio_delay_controller
    COMMAND audio_delay_controller_tests
)
