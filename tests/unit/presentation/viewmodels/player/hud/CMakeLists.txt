add_executable(
    player_hud_message_queue_tests
    hud_message_queue_test.cpp
)

player_apply_compiler_options(player_hud_message_queue_tests)
player_enable_compiler_warnings(player_hud_message_queue_tests)
player_enable_sanitizers(player_hud_message_queue_tests)
player_enable_static_analysis(player_hud_message_queue_tests)

target_link_libraries(
    player_hud_message_queue_tests
    PRIVATE
        player_presentation_hud
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_hud_message_queue
    COMMAND player_hud_message_queue_tests
)
