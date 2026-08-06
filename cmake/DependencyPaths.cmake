include_guard(GLOBAL)

# Reusable third-party packages live directly beside the repository.
# The repository name and its absolute location are irrelevant; every committed
# path starts from the source directory and moves exactly one level up with ../.
#
# Example:
#   ../Qt/<version>/msvc2022_64
#   ../libmpv/windows-x64
#   ../cache/cmake/fetchcontent
#
# CMake may normalize these paths internally while configuring, but source-
# controlled files never store a machine-specific absolute path.
set(
    PLAYER_QT_ROOT
    ""
    CACHE STRING
    "Repository-parent-relative Qt MSVC 2022 x64 kit root discovered under ../Qt"
)

set(
    PLAYER_LIBMPV_ROOT
    "../libmpv/windows-x64"
    CACHE STRING
    "Repository-parent-relative libmpv SDK root; integration begins in R2"
    FORCE
)

set(
    PLAYER_FETCHCONTENT_ROOT
    "../cache/cmake/fetchcontent"
    CACHE STRING
    "Repository-parent-relative shared CMake FetchContent cache"
    FORCE
)

set(_player_parent_root "${CMAKE_SOURCE_DIR}/..")
set(_player_fetchcontent_root "${CMAKE_SOURCE_DIR}/${PLAYER_FETCHCONTENT_ROOT}")
set(FETCHCONTENT_BASE_DIR "${_player_fetchcontent_root}" CACHE PATH "Shared FetchContent base directory" FORCE)

function(player_find_default_qt_root output_variable)
    set(_qt_candidates)

    foreach(_qt_base IN ITEMS
        "${_player_parent_root}/Qt"
        "${_player_parent_root}/qt"
    )
        if(EXISTS "${_qt_base}")
            file(
                GLOB _qt_version_candidates
                LIST_DIRECTORIES TRUE
                "${_qt_base}/*/msvc2022_64"
            )
            list(APPEND _qt_candidates ${_qt_version_candidates})

            if(EXISTS "${_qt_base}/msvc2022_64")
                list(APPEND _qt_candidates "${_qt_base}/msvc2022_64")
            endif()
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _qt_candidates)
    list(SORT _qt_candidates COMPARE NATURAL ORDER DESCENDING)

    foreach(_candidate IN LISTS _qt_candidates)
        if(EXISTS "${_candidate}/lib/cmake/Qt6/Qt6Config.cmake")
            file(RELATIVE_PATH _relative_qt_root "${CMAKE_SOURCE_DIR}" "${_candidate}")
            set(${output_variable} "${_relative_qt_root}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    set(${output_variable} "" PARENT_SCOPE)
endfunction()

if(PLAYER_QT_ROOT STREQUAL "")
    player_find_default_qt_root(_player_qt_root)
    set(
        PLAYER_QT_ROOT
        "${_player_qt_root}"
        CACHE STRING
        "Repository-parent-relative Qt MSVC 2022 x64 kit root discovered under ../Qt"
        FORCE
    )
endif()

if(PLAYER_QT_ROOT STREQUAL "")
    message(FATAL_ERROR
        "Qt 6 was not found in the repository parent directory.\n"
        "Expected: ../Qt/<version>/msvc2022_64\n"
        "Install the Qt MSVC 2022 64-bit kit beside the repository."
    )
endif()

set(_player_qt_root "${CMAKE_SOURCE_DIR}/${PLAYER_QT_ROOT}")

if(NOT EXISTS "${_player_qt_root}/lib/cmake/Qt6/Qt6Config.cmake")
    message(FATAL_ERROR
        "PLAYER_QT_ROOT is invalid: ${PLAYER_QT_ROOT}\n"
        "Expected Qt6Config.cmake under lib/cmake/Qt6.\n"
        "Qt must remain at ../Qt/<version>/msvc2022_64."
    )
endif()

list(PREPEND CMAKE_PREFIX_PATH "${_player_qt_root}")
