add_executable(
    player_screen_structure_tests
    player_screen_structure_test.cpp
)

player_apply_compiler_options(player_screen_structure_tests)
player_enable_compiler_warnings(player_screen_structure_tests)
player_enable_sanitizers(player_screen_structure_tests)
player_enable_static_analysis(player_screen_structure_tests)

add_dependencies(
    player_screen_structure_tests
    player_qml_lint
)

target_compile_definitions(
    player_screen_structure_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_screen_structure_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_screen_structure
    COMMAND player_screen_structure_tests
)

add_executable(
    player_video_viewport_tests
    video_viewport_test.cpp
)

player_apply_compiler_options(player_video_viewport_tests)
player_enable_compiler_warnings(player_video_viewport_tests)
player_enable_sanitizers(player_video_viewport_tests)
player_enable_static_analysis(player_video_viewport_tests)

add_dependencies(
    player_video_viewport_tests
    player_qml_lint
)

target_compile_definitions(
    player_video_viewport_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_video_viewport_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_video_viewport
    COMMAND player_video_viewport_tests
)

add_executable(
    player_top_region_tests
    player_top_region_test.cpp
)

player_apply_compiler_options(player_top_region_tests)
player_enable_compiler_warnings(player_top_region_tests)
player_enable_sanitizers(player_top_region_tests)
player_enable_static_analysis(player_top_region_tests)

add_dependencies(
    player_top_region_tests
    player_qml_lint
)

target_compile_definitions(
    player_top_region_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_top_region_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_top_region
    COMMAND player_top_region_tests
)

add_executable(
    player_bottom_region_tests
    player_bottom_region_test.cpp
)

player_apply_compiler_options(player_bottom_region_tests)
player_enable_compiler_warnings(player_bottom_region_tests)
player_enable_sanitizers(player_bottom_region_tests)
player_enable_static_analysis(player_bottom_region_tests)

target_compile_definitions(
    player_bottom_region_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_bottom_region_tests
    PRIVATE
        Qt6::Test
)

add_test(
    NAME player_bottom_region
    COMMAND player_bottom_region_tests
)

add_subdirectory(media_open)
