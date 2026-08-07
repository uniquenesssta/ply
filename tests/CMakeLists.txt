find_package(
    Qt6 ${PLAYER_QT_VERSION} EXACT
    REQUIRED COMPONENTS
        Test
)

add_executable(
    runtime_paths_tests
    unit/app/bootstrap/runtime_paths_test.cpp
    ../src/app/bootstrap/runtime_paths.cpp
    ../src/app/bootstrap/runtime_paths.h
)

player_apply_compiler_options(runtime_paths_tests)
player_enable_compiler_warnings(runtime_paths_tests)
player_enable_sanitizers(runtime_paths_tests)
player_enable_static_analysis(runtime_paths_tests)

target_include_directories(
    runtime_paths_tests
    PRIVATE
        ../src
)

target_link_libraries(
    runtime_paths_tests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME runtime_paths
    COMMAND runtime_paths_tests
)

add_executable(
    logging_tests
    unit/foundation/logging/logging_test.cpp
)

player_apply_compiler_options(logging_tests)
player_enable_compiler_warnings(logging_tests)
player_enable_sanitizers(logging_tests)
player_enable_static_analysis(logging_tests)

target_link_libraries(
    logging_tests
    PRIVATE
        player_foundation
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME logging
    COMMAND logging_tests
)
