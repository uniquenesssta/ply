add_executable(
    playback_cleanup_matrix_domain_tests
    playback_cleanup_matrix_test.cpp
)

player_apply_compiler_options(playback_cleanup_matrix_domain_tests)
player_enable_compiler_warnings(playback_cleanup_matrix_domain_tests)
player_enable_sanitizers(playback_cleanup_matrix_domain_tests)
player_enable_static_analysis(playback_cleanup_matrix_domain_tests)

target_link_libraries(
    playback_cleanup_matrix_domain_tests
    PRIVATE
        player_playback_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playback_cleanup_matrix
    COMMAND playback_cleanup_matrix_domain_tests
)
