add_executable(
    tracks_delay_controller_tests
    delay_controller_test.cpp
)

player_apply_compiler_options(tracks_delay_controller_tests)
player_enable_compiler_warnings(tracks_delay_controller_tests)
player_enable_sanitizers(tracks_delay_controller_tests)
player_enable_static_analysis(tracks_delay_controller_tests)

target_link_libraries(
    tracks_delay_controller_tests
    PRIVATE
        player_tracks_application
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME tracks_delay_controller
    COMMAND tracks_delay_controller_tests
)

add_executable(
    tracks_track_selection_tests
    track_selection_controller_test.cpp
)

player_apply_compiler_options(tracks_track_selection_tests)
player_enable_compiler_warnings(tracks_track_selection_tests)
player_enable_sanitizers(tracks_track_selection_tests)
player_enable_static_analysis(tracks_track_selection_tests)

target_link_libraries(
    tracks_track_selection_tests
    PRIVATE
        player_tracks_application
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME tracks_track_selection
    COMMAND tracks_track_selection_tests
)
