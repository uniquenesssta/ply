include_guard(GLOBAL)

set(PLAYER_DEVELOPMENT_ROOT_MARKER ".player-development-root")

function(player_generate_development_runtime_marker target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Development runtime marker target does not exist: ${target}")
    endif()

    # The current build contract places Player.exe at build/<preset>/Player.exe.
    # Store only a relative route back to the repository; never persist a
    # machine-specific source path in the generated development artifact.
    file(
        GENERATE
        OUTPUT "$<TARGET_FILE_DIR:${target}>/${PLAYER_DEVELOPMENT_ROOT_MARKER}"
        CONTENT "../..\n"
        TARGET ${target}
    )
endfunction()
