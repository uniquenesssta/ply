include_guard(GLOBAL)

function(player_enable_static_analysis target)
    if(NOT PLAYER_ENABLE_CLANG_TIDY)
        return()
    endif()

    find_program(PLAYER_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
    set_target_properties(
        ${target}
        PROPERTIES
            CXX_CLANG_TIDY "${PLAYER_CLANG_TIDY_EXECUTABLE}"
    )
endfunction()
