include_guard(GLOBAL)

function(player_apply_compiler_options target)
    target_compile_features(${target} PRIVATE cxx_std_${PLAYER_CXX_STANDARD})
    set_target_properties(
        ${target}
        PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
    )
endfunction()
