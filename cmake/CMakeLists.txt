list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

include(DependencyVersions)
include(DependencyPaths)
include(CompilerOptions)
include(CompilerWarnings)
include(Sanitizers)
include(StaticAnalysis)
include(AppTargets)
include(QtRuntimeDeployment)

option(PLAYER_ENABLE_SANITIZERS "Enable supported sanitizers in development builds" OFF)
option(PLAYER_ENABLE_CLANG_TIDY "Enable clang-tidy for project targets" OFF)

find_package(
    Qt6 ${PLAYER_QT_VERSION} EXACT
    REQUIRED COMPONENTS
        Core
        Gui
        Qml
        Quick
        QuickControls2
)

find_package(LibMpv ${PLAYER_MPV_VERSION} EXACT REQUIRED)

qt_standard_project_setup(REQUIRES ${PLAYER_QT_VERSION})

add_subdirectory("../src" "../src")

if(PLAYER_BUILD_TESTS)
    add_subdirectory("../tests" "../tests")
endif()
