add_executable(
    player_transport_view_model_tests
    player_transport_view_model_test.cpp
)

player_apply_compiler_options(player_transport_view_model_tests)
player_enable_compiler_warnings(player_transport_view_model_tests)
player_enable_sanitizers(player_transport_view_model_tests)
player_enable_static_analysis(player_transport_view_model_tests)

target_link_libraries(
    player_transport_view_model_tests
    PRIVATE
        player_presentation_transport
        Qt6::Core
        Qt6::Test
)

add_test(
    NAME player_transport_view_model
    COMMAND player_transport_view_model_tests
)
