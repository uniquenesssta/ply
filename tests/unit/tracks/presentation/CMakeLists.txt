add_executable(
    track_list_model_tests
    track_list_model_test.cpp
)

player_apply_compiler_options(track_list_model_tests)
player_enable_compiler_warnings(track_list_model_tests)
player_enable_sanitizers(track_list_model_tests)
player_enable_static_analysis(track_list_model_tests)

target_link_libraries(
    track_list_model_tests
    PRIVATE
        player_tracks_presentation
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME track_list_model
    COMMAND track_list_model_tests
)

add_executable(
    chapter_model_tests
    chapter_model_test.cpp
)

player_apply_compiler_options(chapter_model_tests)
player_enable_compiler_warnings(chapter_model_tests)
player_enable_sanitizers(chapter_model_tests)
player_enable_static_analysis(chapter_model_tests)

target_link_libraries(
    chapter_model_tests
    PRIVATE
        player_tracks_presentation
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME chapter_model
    COMMAND chapter_model_tests
)
