add_library(player_mpv_infrastructure STATIC)

add_subdirectory(client)
add_subdirectory(initialization)
add_subdirectory(errors)
add_subdirectory(commands)
add_subdirectory(events)
add_subdirectory(properties)
add_subdirectory(runtime)
add_subdirectory(render)

player_apply_compiler_options(player_mpv_infrastructure)
player_enable_compiler_warnings(player_mpv_infrastructure)
player_enable_sanitizers(player_mpv_infrastructure)
player_enable_static_analysis(player_mpv_infrastructure)

target_include_directories(
    player_mpv_infrastructure
    PUBLIC
        ../../..
)

target_link_libraries(
    player_mpv_infrastructure
    PUBLIC
        Qt6::Core
    PRIVATE
        LibMpv::LibMpv
)

get_target_property(_player_libmpv_runtime_path LibMpv::LibMpv IMPORTED_LOCATION)
get_filename_component(_player_libmpv_runtime_filename "${_player_libmpv_runtime_path}" NAME)

target_compile_definitions(
    player_mpv_infrastructure
    PRIVATE
        PLAYER_LIBMPV_RUNTIME_FILENAME="${_player_libmpv_runtime_filename}"
        PLAYER_LIBMPV_STAGED_MANIFEST_RELATIVE_PATH="dependencies/libmpv/dependency-manifest.json"
        PLAYER_EXPECTED_MPV_VERSION="${PLAYER_MPV_VERSION}"
        PLAYER_EXPECTED_MPV_TAG="${PLAYER_MPV_TAG}"
        PLAYER_EXPECTED_MPV_COMMIT="${PLAYER_MPV_COMMIT}"
        PLAYER_EXPECTED_FFMPEG_VERSION="${PLAYER_FFMPEG_VERSION}"
)
