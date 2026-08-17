add_executable(
    playlist_domain_tests
    playlist_test.cpp
)

player_apply_compiler_options(playlist_domain_tests)
player_enable_compiler_warnings(playlist_domain_tests)
player_enable_sanitizers(playlist_domain_tests)
player_enable_static_analysis(playlist_domain_tests)

target_link_libraries(
    playlist_domain_tests
    PRIVATE
        player_playlist_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playlist_domain
    COMMAND playlist_domain_tests
)

add_executable(
    playlist_navigation_tests
    playlist_navigation_test.cpp
)

player_apply_compiler_options(playlist_navigation_tests)
player_enable_compiler_warnings(playlist_navigation_tests)
player_enable_sanitizers(playlist_navigation_tests)
player_enable_static_analysis(playlist_navigation_tests)

target_link_libraries(
    playlist_navigation_tests
    PRIVATE
        player_playlist_domain
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playlist_navigation
    COMMAND playlist_navigation_tests
)
