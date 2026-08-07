find_package(
    Qt6 ${PLAYER_QT_VERSION} EXACT
    REQUIRED COMPONENTS
        Test
)

add_executable(
    runtime_paths_tests
    unit/app/bootstrap/runtime_paths_test.cpp
    ${PROJECT_SOURCE_DIR}/src/app/bootstrap/runtime_paths.cpp
    ${PROJECT_SOURCE_DIR}/src/app/bootstrap/runtime_paths.h
)

player_apply_compiler_options(runtime_paths_tests)
player_enable_compiler_warnings(runtime_paths_tests)
player_enable_sanitizers(runtime_paths_tests)
player_enable_static_analysis(runtime_paths_tests)

target_include_directories(
    runtime_paths_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
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
