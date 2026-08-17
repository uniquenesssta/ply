add_executable(
    playlist_application_tests
    playlist_application_test.cpp
)

player_apply_compiler_options(playlist_application_tests)
player_enable_compiler_warnings(playlist_application_tests)
player_enable_sanitizers(playlist_application_tests)
player_enable_static_analysis(playlist_application_tests)

target_link_libraries(
    playlist_application_tests
    PRIVATE
        player_playlist_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playlist_application
    COMMAND playlist_application_tests
)

add_executable(
    playlist_auto_advance_tests
    playlist_auto_advance_test.cpp
)

player_apply_compiler_options(playlist_auto_advance_tests)
player_enable_compiler_warnings(playlist_auto_advance_tests)
player_enable_sanitizers(playlist_auto_advance_tests)
player_enable_static_analysis(playlist_auto_advance_tests)

target_link_libraries(
    playlist_auto_advance_tests
    PRIVATE
        player_playlist_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playlist_auto_advance
    COMMAND playlist_auto_advance_tests
)
