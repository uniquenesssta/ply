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

add_executable(
    graphics_backend_tests
    unit/app/bootstrap/graphics_backend/graphics_backend_probe_test.cpp
    ../src/app/bootstrap/graphics_backend/graphics_backend_bootstrap.cpp
    ../src/app/bootstrap/graphics_backend/graphics_backend_bootstrap.h
    ../src/app/bootstrap/graphics_backend/graphics_backend_probe.cpp
    ../src/app/bootstrap/graphics_backend/graphics_backend_probe.h
)

player_apply_compiler_options(graphics_backend_tests)
player_enable_compiler_warnings(graphics_backend_tests)
player_enable_sanitizers(graphics_backend_tests)
player_enable_static_analysis(graphics_backend_tests)

target_include_directories(
    graphics_backend_tests
    PRIVATE
        ../src
)

target_link_libraries(
    graphics_backend_tests
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Test
)

add_test(
    NAME graphics_backend
    COMMAND graphics_backend_tests
)

add_executable(
    application_container_tests
    unit/app/composition/application_container_test.cpp
    ../src/app/composition/application_container.cpp
    ../src/app/composition/application_container.h
    ../src/app/bootstrap/logging_bootstrap.cpp
    ../src/app/bootstrap/logging_bootstrap.h
    ../src/app/bootstrap/qml_bootstrap.cpp
    ../src/app/bootstrap/qml_bootstrap.h
    ../src/app/bootstrap/runtime_paths.cpp
    ../src/app/bootstrap/runtime_paths.h
)

player_apply_compiler_options(application_container_tests)
player_enable_compiler_warnings(application_container_tests)
player_enable_sanitizers(application_container_tests)
player_enable_static_analysis(application_container_tests)

target_include_directories(
    application_container_tests
    PRIVATE
        ../src
)

target_link_libraries(
    application_container_tests
    PRIVATE
        player_foundation
        Qt6::Core
        Qt6::Qml
        Qt6::Test
)

add_test(
    NAME application_container
    COMMAND application_container_tests
)

add_executable(
    mpv_handle_tests
    unit/playback/infrastructure/mpv/client/mpv_handle_test.cpp
)

player_apply_compiler_options(mpv_handle_tests)
player_enable_compiler_warnings(mpv_handle_tests)
player_enable_sanitizers(mpv_handle_tests)
player_enable_static_analysis(mpv_handle_tests)

target_link_libraries(
    mpv_handle_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_handle_tests)

add_test(
    NAME mpv_handle
    COMMAND mpv_handle_tests
)

add_executable(
    mpv_initialization_tests
    unit/playback/infrastructure/mpv/initialization/mpv_initializer_test.cpp
)

player_apply_compiler_options(mpv_initialization_tests)
player_enable_compiler_warnings(mpv_initialization_tests)
player_enable_sanitizers(mpv_initialization_tests)
player_enable_static_analysis(mpv_initialization_tests)

target_link_libraries(
    mpv_initialization_tests
    PRIVATE
        player_mpv_infrastructure
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_initialization_tests)

add_test(
    NAME mpv_initialization
    COMMAND mpv_initialization_tests
)

add_executable(
    mpv_event_loop_tests
    unit/playback/infrastructure/mpv/events/mpv_event_loop_test.cpp
)

player_apply_compiler_options(mpv_event_loop_tests)
player_enable_compiler_warnings(mpv_event_loop_tests)
player_enable_sanitizers(mpv_event_loop_tests)
player_enable_static_analysis(mpv_event_loop_tests)

target_link_libraries(
    mpv_event_loop_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_event_loop_tests)

add_test(
    NAME mpv_event_loop
    COMMAND mpv_event_loop_tests
)

add_executable(
    mpv_command_tests
    unit/playback/infrastructure/mpv/commands/mpv_command_executor_test.cpp
)

player_apply_compiler_options(mpv_command_tests)
player_enable_compiler_warnings(mpv_command_tests)
player_enable_sanitizers(mpv_command_tests)
player_enable_static_analysis(mpv_command_tests)

target_link_libraries(
    mpv_command_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_command_tests)

add_test(
    NAME mpv_commands
    COMMAND mpv_command_tests
)

add_executable(
    mpv_property_tests
    unit/playback/infrastructure/mpv/properties/mpv_property_observer_test.cpp
)

player_apply_compiler_options(mpv_property_tests)
player_enable_compiler_warnings(mpv_property_tests)
player_enable_sanitizers(mpv_property_tests)
player_enable_static_analysis(mpv_property_tests)

target_link_libraries(
    mpv_property_tests
    PRIVATE
        player_mpv_infrastructure
        LibMpv::LibMpv
        Qt6::Core
        Qt6::Test
)

player_stage_libmpv_runtime(mpv_property_tests)

add_test(
    NAME mpv_properties
    COMMAND mpv_property_tests
)

add_executable(
    mpv_runtime_probe_tests
    unit/playback/infrastructure/mpv/runtime/mpv_runtime_probe_test.cpp
)

player_apply_compiler_options(mpv_runtime_probe_tests)
player_enable_compiler_warnings(mpv_runtime_probe_tests)
player_enable_sanitizers(mpv_runtime_probe_tests)
player_enable_static_analysis(mpv_runtime_probe_tests)

target_link_libraries(
    mpv_runtime_probe_tests
    PRIVATE
        player_mpv_infrastructure
        Qt6::Test
)

target_compile_definitions(
    mpv_runtime_probe_tests
    PRIVATE
        PLAYER_EXPECTED_MPV_VERSION="${PLAYER_MPV_VERSION}"
        PLAYER_EXPECTED_MPV_TAG="${PLAYER_MPV_TAG}"
        PLAYER_EXPECTED_MPV_COMMIT="${PLAYER_MPV_COMMIT}"
        PLAYER_EXPECTED_FFMPEG_VERSION="${PLAYER_FFMPEG_VERSION}"
)

player_stage_libmpv_runtime(mpv_runtime_probe_tests)

add_test(
    NAME mpv_runtime_probe
    COMMAND mpv_runtime_probe_tests
)
