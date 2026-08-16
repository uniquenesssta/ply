add_executable(
    player_media_open_tests
    player_media_open_test.cpp
)

player_apply_compiler_options(player_media_open_tests)
player_enable_compiler_warnings(player_media_open_tests)
player_enable_sanitizers(player_media_open_tests)
player_enable_static_analysis(player_media_open_tests)

add_dependencies(
    player_media_open_tests
    player_qml_lint
)

target_compile_definitions(
    player_media_open_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_media_open_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_media_open
    COMMAND player_media_open_tests
)

add_executable(
    player_url_open_tests
    player_url_open_test.cpp
)

player_apply_compiler_options(player_url_open_tests)
player_enable_compiler_warnings(player_url_open_tests)
player_enable_sanitizers(player_url_open_tests)
player_enable_static_analysis(player_url_open_tests)

add_dependencies(
    player_url_open_tests
    player_qml_lint
)

target_compile_definitions(
    player_url_open_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_url_open_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_url_open
    COMMAND player_url_open_tests
)
