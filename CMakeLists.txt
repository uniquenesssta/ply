cmake_minimum_required(VERSION 3.25)

project(
    ModularMpvPlayer
    VERSION 0.1.0
    DESCRIPTION "A modular Qt 6 desktop player shell prepared for libmpv integration"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DependencyPaths)
include(DependencyVersions)
include(CompilerOptions)
include(CompilerWarnings)
include(Sanitizers)
include(StaticAnalysis)
include(AppTargets)

option(PLAYER_BUILD_TESTS "Build player tests" ON)
option(PLAYER_ENABLE_SANITIZERS "Enable supported sanitizers in development builds" OFF)
option(PLAYER_ENABLE_CLANG_TIDY "Enable clang-tidy for project targets" OFF)

find_package(
    Qt6 ${PLAYER_QT_MIN_VERSION}
    REQUIRED COMPONENTS
        Core
        Gui
        Qml
        Quick
        QuickControls2
)

qt_standard_project_setup(REQUIRES ${PLAYER_QT_MIN_VERSION})

add_subdirectory(src)

if(PLAYER_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
