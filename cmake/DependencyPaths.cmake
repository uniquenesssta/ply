include_guard(GLOBAL)

foreach(_required_version_variable IN ITEMS
    PLAYER_QT_VERSION
    PLAYER_MPV_VERSION
)
    if(NOT DEFINED ${_required_version_variable} OR "${${_required_version_variable}}" STREQUAL "")
        message(FATAL_ERROR
            "${_required_version_variable} must be defined by cmake/DependencyVersions.cmake before DependencyPaths is included."
        )
    endif()
endforeach()

# The repository parent is the dependency workspace. Only the actual sibling
# directories are represented here: Qt, libmpv, downloads, and cache.
# CMake/Ninja are development tools discovered by PowerShell from PATH or the
# official Qt tools tree below ../Qt/Tools; they are not extra sibling roots.
set(PLAYER_QT_ROOT "../Qt/${PLAYER_QT_VERSION}/msvc2022_64" CACHE INTERNAL "Pinned Qt kit root")
set(PLAYER_LIBMPV_ROOT "../libmpv/${PLAYER_MPV_VERSION}/windows-x64" CACHE INTERNAL "Pinned libmpv SDK/runtime root")
set(PLAYER_DOWNLOADS_ROOT "../downloads" CACHE INTERNAL "Shared dependency download archive root")
set(PLAYER_FETCHCONTENT_ROOT "../cache/cmake/fetchcontent" CACHE INTERNAL "Shared CMake FetchContent cache")

function(player_validate_dependency_path variable_name)
    set(_path "${${variable_name}}")
    if(IS_ABSOLUTE "${_path}" OR NOT "${_path}" MATCHES "^\\.\\./")
        message(FATAL_ERROR
            "${variable_name} must be repository-parent-relative and begin with ../; received '${_path}'."
        )
    endif()
endfunction()

foreach(_dependency_path_variable IN ITEMS
    PLAYER_QT_ROOT
    PLAYER_LIBMPV_ROOT
    PLAYER_DOWNLOADS_ROOT
    PLAYER_FETCHCONTENT_ROOT
)
    player_validate_dependency_path(${_dependency_path_variable})
endforeach()

# CMake resolves these relative locations at configure time. Source control
# stores only the relative forms above, so moving the workspace to another
# drive or parent directory does not require configuration edits.
set(_player_qt_root "${CMAKE_SOURCE_DIR}/${PLAYER_QT_ROOT}")
set(_player_fetchcontent_root "${CMAKE_SOURCE_DIR}/${PLAYER_FETCHCONTENT_ROOT}")
set(FETCHCONTENT_BASE_DIR "${_player_fetchcontent_root}" CACHE PATH "Shared FetchContent base directory" FORCE)

if(NOT EXISTS "${_player_qt_root}/lib/cmake/Qt6/Qt6Config.cmake")
    message(FATAL_ERROR
        "Pinned Qt ${PLAYER_QT_VERSION} MSVC 2022 x64 kit was not found.\n"
        "Expected relative path: ${PLAYER_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake"
    )
endif()

list(PREPEND CMAKE_PREFIX_PATH "${_player_qt_root}")
