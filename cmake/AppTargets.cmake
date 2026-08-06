include_guard(GLOBAL)

function(player_configure_application_target target)
    player_apply_compiler_options(${target})
    player_enable_compiler_warnings(${target})
    player_enable_sanitizers(${target})
    player_enable_static_analysis(${target})

    target_link_libraries(
        ${target}
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Qml
            Qt6::Quick
            Qt6::QuickControls2
    )

    set_target_properties(
        ${target}
        PROPERTIES
            OUTPUT_NAME "Player"
    )
endfunction()
