add_executable(
    media_open_coordinator_tests
    media_open_coordinator_test.cpp
)

player_apply_compiler_options(media_open_coordinator_tests)
player_enable_compiler_warnings(media_open_coordinator_tests)
player_enable_sanitizers(media_open_coordinator_tests)
player_enable_static_analysis(media_open_coordinator_tests)

target_link_libraries(
    media_open_coordinator_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME media_open_coordinator
    COMMAND media_open_coordinator_tests
)

add_executable(
    media_open_playback_integration_tests
    media_open_playback_integration_test.cpp
)

player_apply_compiler_options(media_open_playback_integration_tests)
player_enable_compiler_warnings(media_open_playback_integration_tests)
player_enable_sanitizers(media_open_playback_integration_tests)
player_enable_static_analysis(media_open_playback_integration_tests)

target_link_libraries(
    media_open_playback_integration_tests
    PRIVATE
        player_media_application
        player_playback_application
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(media_open_playback_integration_tests)

add_test(
    NAME media_open_playback_integration
    COMMAND media_open_playback_integration_tests
)

add_executable(
    media_drop_handler_tests
    media_drop_handler_test.cpp
)

player_apply_compiler_options(media_drop_handler_tests)
player_enable_compiler_warnings(media_drop_handler_tests)
player_enable_sanitizers(media_drop_handler_tests)
player_enable_static_analysis(media_drop_handler_tests)

target_link_libraries(
    media_drop_handler_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME media_drop_handler
    COMMAND media_drop_handler_tests
)

add_executable(
    url_media_validator_tests
    url_media_validator_test.cpp
)

player_apply_compiler_options(url_media_validator_tests)
player_enable_compiler_warnings(url_media_validator_tests)
player_enable_sanitizers(url_media_validator_tests)
player_enable_static_analysis(url_media_validator_tests)

target_link_libraries(
    url_media_validator_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME url_media_validator
    COMMAND url_media_validator_tests
)

add_executable(
    url_open_workflow_tests
    url_open_workflow_test.cpp
)

player_apply_compiler_options(url_open_workflow_tests)
player_enable_compiler_warnings(url_open_workflow_tests)
player_enable_sanitizers(url_open_workflow_tests)
player_enable_static_analysis(url_open_workflow_tests)

target_link_libraries(
    url_open_workflow_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME url_open_workflow
    COMMAND url_open_workflow_tests
)

add_executable(
    media_argument_parser_tests
    media_argument_parser_test.cpp
)

player_apply_compiler_options(media_argument_parser_tests)
player_enable_compiler_warnings(media_argument_parser_tests)
player_enable_sanitizers(media_argument_parser_tests)
player_enable_static_analysis(media_argument_parser_tests)

target_link_libraries(
    media_argument_parser_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME media_argument_parser
    COMMAND media_argument_parser_tests
)

add_executable(
    media_argument_open_workflow_tests
    media_argument_open_workflow_test.cpp
)

player_apply_compiler_options(media_argument_open_workflow_tests)
player_enable_compiler_warnings(media_argument_open_workflow_tests)
player_enable_sanitizers(media_argument_open_workflow_tests)
player_enable_static_analysis(media_argument_open_workflow_tests)

target_link_libraries(
    media_argument_open_workflow_tests
    PRIVATE
        player_media_application
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME media_argument_open_workflow
    COMMAND media_argument_open_workflow_tests
)
