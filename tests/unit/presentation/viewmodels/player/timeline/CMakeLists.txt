add_executable(
    timeline_scrub_session_tests
    timeline_scrub_session_test.cpp
)

player_apply_compiler_options(timeline_scrub_session_tests)
player_enable_compiler_warnings(timeline_scrub_session_tests)
player_enable_sanitizers(timeline_scrub_session_tests)
player_enable_static_analysis(timeline_scrub_session_tests)

target_link_libraries(
    timeline_scrub_session_tests
    PRIVATE
        player_presentation_timeline
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME timeline_scrub_session
    COMMAND timeline_scrub_session_tests
)

add_executable(
    player_timeline_view_model_tests
    player_timeline_view_model_test.cpp
)

player_apply_compiler_options(player_timeline_view_model_tests)
player_enable_compiler_warnings(player_timeline_view_model_tests)
player_enable_sanitizers(player_timeline_view_model_tests)
player_enable_static_analysis(player_timeline_view_model_tests)

target_link_libraries(
    player_timeline_view_model_tests
    PRIVATE
        player_presentation_timeline
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_timeline_view_model
    COMMAND player_timeline_view_model_tests
)

add_executable(
    timeline_interaction_matrix_tests
    timeline_interaction_matrix_test.cpp
)

player_apply_compiler_options(timeline_interaction_matrix_tests)
player_enable_compiler_warnings(timeline_interaction_matrix_tests)
player_enable_sanitizers(timeline_interaction_matrix_tests)
player_enable_static_analysis(timeline_interaction_matrix_tests)

target_link_libraries(
    timeline_interaction_matrix_tests
    PRIVATE
        player_presentation_timeline
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME timeline_interaction_matrix
    COMMAND timeline_interaction_matrix_tests
)
