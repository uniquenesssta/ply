add_executable(
    player_playlist_qml_tests
    playlist_qml_test.cpp
)

player_apply_compiler_options(player_playlist_qml_tests)
player_enable_compiler_warnings(player_playlist_qml_tests)
player_enable_sanitizers(player_playlist_qml_tests)
player_enable_static_analysis(player_playlist_qml_tests)

target_compile_definitions(
    player_playlist_qml_tests
    PRIVATE
        PLAYER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    player_playlist_qml_tests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_playlist_qml
    COMMAND player_playlist_qml_tests
)
