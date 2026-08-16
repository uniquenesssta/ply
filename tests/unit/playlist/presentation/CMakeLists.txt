add_executable(
    playlist_list_model_tests
    playlist_list_model_test.cpp
)

player_apply_compiler_options(playlist_list_model_tests)
player_enable_compiler_warnings(playlist_list_model_tests)
player_enable_sanitizers(playlist_list_model_tests)
player_enable_static_analysis(playlist_list_model_tests)

target_link_libraries(
    playlist_list_model_tests
    PRIVATE
        player_playlist_presentation
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME playlist_list_model
    COMMAND playlist_list_model_tests
)
