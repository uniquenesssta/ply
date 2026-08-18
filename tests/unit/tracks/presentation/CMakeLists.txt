add_executable(
    tracks_presentation_tests
    track_list_model_test.cpp
)

player_apply_compiler_options(tracks_presentation_tests)
player_enable_compiler_warnings(tracks_presentation_tests)
player_enable_sanitizers(tracks_presentation_tests)
player_enable_static_analysis(tracks_presentation_tests)

target_link_libraries(
    tracks_presentation_tests
    PRIVATE
        player_tracks_presentation
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME tracks_presentation
    COMMAND tracks_presentation_tests
)
