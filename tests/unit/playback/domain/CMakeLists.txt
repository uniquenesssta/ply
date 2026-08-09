add_executable(
    playback_command_domain_tests
    commands/playback_command_test.cpp
)

player_apply_compiler_options(playback_command_domain_tests)
player_enable_compiler_warnings(playback_command_domain_tests)
player_enable_sanitizers(playback_command_domain_tests)
player_enable_static_analysis(playback_command_domain_tests)

target_link_libraries(
    playback_command_domain_tests
    PRIVATE
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_commands
    COMMAND playback_command_domain_tests
)

add_executable(
    playback_event_mapping_tests
    events/playback_event_mapping_test.cpp
)

player_apply_compiler_options(playback_event_mapping_tests)
player_enable_compiler_warnings(playback_event_mapping_tests)
player_enable_sanitizers(playback_event_mapping_tests)
player_enable_static_analysis(playback_event_mapping_tests)

target_link_libraries(
    playback_event_mapping_tests
    PRIVATE
        player_playback_domain
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_events
    COMMAND playback_event_mapping_tests
)

add_executable(
    playback_media_state_mapping_tests
    events/mpv_media_model_mapper_test.cpp
)

player_apply_compiler_options(playback_media_state_mapping_tests)
player_enable_compiler_warnings(playback_media_state_mapping_tests)
player_enable_sanitizers(playback_media_state_mapping_tests)
player_enable_static_analysis(playback_media_state_mapping_tests)

target_link_libraries(
    playback_media_state_mapping_tests
    PRIVATE
        player_playback_domain
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_media_state_mapping
    COMMAND playback_media_state_mapping_tests
)

add_executable(
    playback_snapshot_domain_tests
    state/playback_snapshot_test.cpp
)

player_apply_compiler_options(playback_snapshot_domain_tests)
player_enable_compiler_warnings(playback_snapshot_domain_tests)
player_enable_sanitizers(playback_snapshot_domain_tests)
player_enable_static_analysis(playback_snapshot_domain_tests)

target_link_libraries(
    playback_snapshot_domain_tests
    PRIVATE
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_snapshot
    COMMAND playback_snapshot_domain_tests
)

add_executable(
    playback_reducer_domain_tests
    state/playback_reducer_test.cpp
)

player_apply_compiler_options(playback_reducer_domain_tests)
player_enable_compiler_warnings(playback_reducer_domain_tests)
player_enable_sanitizers(playback_reducer_domain_tests)
player_enable_static_analysis(playback_reducer_domain_tests)

target_link_libraries(
    playback_reducer_domain_tests
    PRIVATE
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_reducer
    COMMAND playback_reducer_domain_tests
)

add_executable(
    playback_invariants_domain_tests
    state/playback_invariants_test.cpp
)

player_apply_compiler_options(playback_invariants_domain_tests)
player_enable_compiler_warnings(playback_invariants_domain_tests)
player_enable_sanitizers(playback_invariants_domain_tests)
player_enable_static_analysis(playback_invariants_domain_tests)

target_link_libraries(
    playback_invariants_domain_tests
    PRIVATE
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_invariants
    COMMAND playback_invariants_domain_tests
)

add_subdirectory(state/reducer)
