include_guard(GLOBAL)

function(player_enable_qt_runtime_deployment target qml_source_dir)
    if(NOT WIN32)
        return()
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR
            "Qt runtime deployment target '${target}' does not exist."
        )
    endif()

    if(NOT DEFINED PLAYER_QT_ROOT OR "${PLAYER_QT_ROOT}" STREQUAL "")
        message(FATAL_ERROR
            "PLAYER_QT_ROOT must be defined before Qt runtime deployment is configured."
        )
    endif()

    if(NOT IS_ABSOLUTE "${qml_source_dir}" OR NOT IS_DIRECTORY "${qml_source_dir}")
        message(FATAL_ERROR
            "Qt runtime deployment requires an existing absolute QML source directory; received '${qml_source_dir}'."
        )
    endif()

    set(_qt_root "${CMAKE_SOURCE_DIR}/${PLAYER_QT_ROOT}")
    cmake_path(NORMAL_PATH _qt_root)
    set(_windeployqt "${_qt_root}/bin/windeployqt.exe")

    if(NOT EXISTS "${_windeployqt}")
        message(FATAL_ERROR
            "Pinned Qt deployment tool was not found at '${_windeployqt}'."
        )
    endif()

    set(_deploy_target "${target}_qt_runtime_deploy")
    if(TARGET ${_deploy_target})
        message(FATAL_ERROR
            "Qt runtime deployment target '${_deploy_target}' already exists."
        )
    endif()

    # Keep build-tree executables directly runnable without mutating PATH or
    # requiring the developer shell. windeployqt owns Qt DLL/plugin/QML import
    # discovery; libmpv runtime staging remains owned by FindLibMpv.cmake.
    add_custom_target(
        ${_deploy_target}
        ALL
        COMMAND
            "${_windeployqt}"
            "--$<IF:$<CONFIG:Debug>,debug,release>"
            --no-translations
            --qmldir "${qml_source_dir}"
            --dir "$<TARGET_FILE_DIR:${target}>"
            "$<TARGET_FILE:${target}>"
        DEPENDS ${target}
        VERBATIM
        COMMENT "Deploying Qt runtime and QML imports for ${target}"
    )
endfunction()
