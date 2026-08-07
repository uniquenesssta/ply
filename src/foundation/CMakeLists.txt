add_library(player_foundation STATIC)

target_include_directories(
    player_foundation
    PUBLIC
        ..
)

target_link_libraries(
    player_foundation
    PUBLIC
        Qt6::Core
)

player_apply_compiler_options(player_foundation)
player_enable_compiler_warnings(player_foundation)
player_enable_sanitizers(player_foundation)
player_enable_static_analysis(player_foundation)

add_subdirectory(logging)
