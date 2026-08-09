param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = (Resolve-Path -LiteralPath $ProjectRoot).Path

function Assert-FileExists {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Required file is missing: $Path"
    }
}

function Assert-DirectoryExists {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "Required directory is missing: $Path"
    }
}

foreach ($file in @(
    "ALL_AI_CODE.md",
    "AI_PROJECT_RULES.md",
    "CMakeLists.txt",
    "CMakePresets.json",
    "README.md",
    ".gitignore",
    "cmake/CMakeLists.txt",
    "cmake/AppTargets.cmake",
    "cmake/CompilerOptions.cmake",
    "cmake/CompilerWarnings.cmake",
    "cmake/DependencyPaths.cmake",
    "cmake/DependencyVersions.cmake",
    "cmake/LibMpv.cmake",
    "cmake/RuntimeDeployment.cmake",
    "cmake/Sanitizers.cmake",
    "cmake/StaticAnalysis.cmake",
    "scripts/build.ps1",
    "scripts/configure.ps1",
    "scripts/launch.ps1",
    "scripts/test.ps1",
    "scripts/verify-dependencies.ps1",
    "scripts/verify-project-layout.ps1",
    "scripts/modules/DependencyPaths.psm1",
    "scripts/modules/DependencyVersions.psm1",
    "scripts/modules/DevelopmentRuntime.psm1",
    "scripts/modules/QtRuntimeDeployment.psm1",
    "src/CMakeLists.txt",
    "src/app/CMakeLists.txt",
    "src/app/main.cpp",
    "src/app/bootstrap/application_bootstrap.cpp",
    "src/app/bootstrap/application_bootstrap.h",
    "src/app/bootstrap/graphics_backend/CMakeLists.txt",
    "src/app/bootstrap/graphics_backend/graphics_backend.cpp",
    "src/app/bootstrap/graphics_backend/graphics_backend.h",
    "src/app/bootstrap/graphics_backend/opengl_backend.cpp",
    "src/app/bootstrap/graphics_backend/opengl_backend.h",
    "src/app/bootstrap/logging_bootstrap.cpp",
    "src/app/bootstrap/logging_bootstrap.h",
    "src/app/bootstrap/qml_bootstrap.cpp",
    "src/app/bootstrap/qml_bootstrap.h",
    "src/app/bootstrap/runtime_paths.cpp",
    "src/app/bootstrap/runtime_paths.h",
    "src/app/composition/application_container.cpp",
    "src/app/composition/application_container.h",
    "src/foundation/CMakeLists.txt",
    "src/foundation/logging/file_log_sink.cpp",
    "src/foundation/logging/file_log_sink.h",
    "src/foundation/logging/logging_categories.cpp",
    "src/foundation/logging/logging_categories.h",
    "src/playback/CMakeLists.txt",
    "src/playback/application/CMakeLists.txt",
    "src/playback/domain/CMakeLists.txt",
    "src/playback/infrastructure/CMakeLists.txt",
    "src/playback/infrastructure/mpv/CMakeLists.txt",
    "src/presentation/CMakeLists.txt",
    "src/presentation/qml/App.qml",
    "src/presentation/qml/MainWindow.qml",
    "src/presentation/qml/qmldir",
    "src/presentation/qml/screens/player/PlayerChrome.qml",
    "src/presentation/qml/screens/player/PlayerScreen.qml",
    "src/presentation/qml/screens/player/VideoSurface.qml",
    "tests/CMakeLists.txt",
    "tests/integration/CMakeLists.txt",
    "tests/integration/app/CMakeLists.txt",
    "tests/integration/app/application_container_test.cpp",
    "tests/unit/CMakeLists.txt",
    "tests/unit/app/CMakeLists.txt",
    "tests/unit/app/graphics_backend/CMakeLists.txt",
    "tests/unit/app/graphics_backend/graphics_backend_test.cpp",
    "tests/unit/foundation/CMakeLists.txt",
    "tests/unit/foundation/logging/CMakeLists.txt",
    "tests/unit/foundation/logging/file_log_sink_test.cpp",
    "tests/unit/playback/CMakeLists.txt",
    "tests/unit/presentation/CMakeLists.txt"
)) {
    Assert-FileExists -Path (Join-Path $projectRoot $file)
}

foreach ($directory in @(
    "cmake",
    "docs",
    "scripts",
    "scripts/modules",
    "src",
    "src/app",
    "src/app/bootstrap",
    "src/app/bootstrap/graphics_backend",
    "src/app/composition",
    "src/foundation",
    "src/foundation/logging",
    "src/playback",
    "src/playback/application",
    "src/playback/domain",
    "src/playback/infrastructure",
    "src/playback/infrastructure/mpv",
    "src/presentation",
    "src/presentation/qml",
    "src/presentation/qml/screens",
    "src/presentation/qml/screens/player",
    "tests",
    "tests/integration",
    "tests/integration/app",
    "tests/unit",
    "tests/unit/app",
    "tests/unit/app/graphics_backend",
    "tests/unit/foundation",
    "tests/unit/foundation/logging",
    "tests/unit/playback",
    "tests/unit/presentation"
)) {
    Assert-DirectoryExists -Path (Join-Path $projectRoot $directory)
}

$cmakeLists = Get-Content -LiteralPath (Join-Path $projectRoot "CMakeLists.txt") -Raw
if (-not $cmakeLists.Contains('add_subdirectory(cmake)')) {
    throw "Root CMakeLists.txt must delegate configuration to cmake/."
}

$cmakeEntry = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'include(DependencyVersions)',
    'include(DependencyPaths)',
    'include(CompilerOptions)',
    'include(CompilerWarnings)',
    'include(Sanitizers)',
    'include(StaticAnalysis)',
    'include(AppTargets)',
    'find_package(',
    'Qt6 ${PLAYER_QT_VERSION} EXACT',
    'find_package(LibMpv ${PLAYER_MPV_VERSION} EXACT REQUIRED)',
    'qt_standard_project_setup(REQUIRES ${PLAYER_QT_VERSION})',
    'add_subdirectory("../src" "../src")'
)) {
    if (-not $cmakeEntry.Contains($fragment)) {
        throw "cmake/CMakeLists.txt is missing required configuration fragment: $fragment"
    }
}

$dependencyVersions = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/DependencyVersions.cmake") -Raw
foreach ($fragment in @(
    'set(PLAYER_CMAKE_MINIMUM_VERSION "3.30.5")',
    'set(PLAYER_NINJA_MINIMUM_VERSION "1.12.1")',
    'set(PLAYER_QT_VERSION "6.8.3")',
    'set(PLAYER_MPV_VERSION "0.41.0")',
    'set(PLAYER_MSVC_COMPILER_FAMILY "19.44")',
    'set(PLAYER_MSVC_TOOLSET_FAMILY "14.44")',
    'set(PLAYER_WINDOWS_SDK_MINIMUM_VERSION "10.0.26100.0")',
    'set(PLAYER_WINDOWS_MINIMUM_BUILD "10.0.19045")'
)) {
    if (-not $dependencyVersions.Contains($fragment)) {
        throw "DependencyVersions.cmake is missing required baseline fragment: $fragment"
    }
}

$dependencyPaths = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/DependencyPaths.cmake") -Raw
foreach ($fragment in @(
    'PLAYER_QT_ROOT',
    'PLAYER_MPV_ROOT',
    'PLAYER_FETCHCONTENT_BASE_DIR',
    'PLAYER_DOWNLOADS_DIR'
)) {
    if (-not $dependencyPaths.Contains($fragment)) {
        throw "DependencyPaths.cmake is missing required path contract fragment: $fragment"
    }
}
if ($dependencyPaths -match '[A-Za-z]:[/\\]') {
    throw "DependencyPaths.cmake contains a machine-absolute Windows path."
}

$appTargets = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/AppTargets.cmake") -Raw
foreach ($fragment in @(
    'function(player_configure_application_target target)',
    'Qt6::Core',
    'Qt6::Gui',
    'Qt6::Qml',
    'Qt6::Quick',
    'Qt6::QuickControls2',
    'OUTPUT_NAME "Player"'
)) {
    if (-not $appTargets.Contains($fragment)) {
        throw "AppTargets.cmake is missing required application target behavior: $fragment"
    }
}

$srcCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'qt_add_executable(player_app MANUAL_FINALIZATION)',
    'add_subdirectory(foundation)',
    'add_subdirectory(playback)',
    'add_subdirectory(app)',
    'add_subdirectory(presentation)',
    'player_configure_application_target(player_app)',
    'player_stage_libmpv_runtime(player_app)',
    'qt_finalize_executable(player_app)'
)) {
    if (-not $srcCMake.Contains($fragment)) {
        throw "src/CMakeLists.txt is missing required application composition fragment: $fragment"
    }
}

$appCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'main.cpp',
    'bootstrap/application_bootstrap.cpp',
    'bootstrap/logging_bootstrap.cpp',
    'bootstrap/qml_bootstrap.cpp',
    'bootstrap/runtime_paths.cpp',
    'add_subdirectory(bootstrap/graphics_backend)',
    'add_subdirectory(composition)',
    'player_foundation',
    'player_mpv_infrastructure'
)) {
    if (-not $appCMake.Contains($fragment)) {
        throw "src/app/CMakeLists.txt is missing required application module fragment: $fragment"
    }
}

$applicationBootstrap = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/application_bootstrap.cpp") -Raw
foreach ($fragment in @(
    'configureOpenGlBackend()',
    'runtimePaths_.initialize(application)',
    'logging_.initialize(runtimePaths_)',
    'graphicsBackend_.initialize()',
    'container_.initialize()',
    'qml_.initialize()',
    'qml_.loadMainModule()',
    'container_.shutdown()',
    'qml_.shutdown()',
    'logging_.shutdown()'
)) {
    if (-not $applicationBootstrap.Contains($fragment)) {
        throw "ApplicationBootstrap is missing required bootstrap order fragment: $fragment"
    }
}

$runtimePathsHeader = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/runtime_paths.h") -Raw
foreach ($fragment in @(
    'class RuntimePaths final',
    'bool initialize(const QCoreApplication& application)',
    'QString applicationDirectory() const',
    'QString projectRoot() const',
    'QString logFilePath() const'
)) {
    if (-not $runtimePathsHeader.Contains($fragment)) {
        throw "RuntimePaths header is missing required path boundary fragment: $fragment"
    }
}

$runtimePathsSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/runtime_paths.cpp") -Raw
foreach ($fragment in @(
    '.player-development-root',
    'QDir::isRelativePath',
    'QFileInfo::exists',
    'canonicalPath()',
    'qCritical()',
    'player.log'
)) {
    if (-not $runtimePathsSource.Contains($fragment)) {
        throw "RuntimePaths source is missing required development-path validation fragment: $fragment"
    }
}
if ($runtimePathsSource -match '[A-Za-z]:[/\\]') {
    throw "RuntimePaths contains a machine-absolute Windows path."
}

$graphicsBackendCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/graphics_backend/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'graphics_backend.cpp',
    'graphics_backend.h',
    'opengl_backend.cpp',
    'opengl_backend.h',
    'Qt6::Gui'
)) {
    if (-not $graphicsBackendCMake.Contains($fragment)) {
        throw "graphics_backend/CMakeLists.txt is missing required graphics backend fragment: $fragment"
    }
}

$graphicsBackendSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/graphics_backend/graphics_backend.cpp") -Raw
foreach ($fragment in @(
    'OpenGlBackend::configureQtQuickBackend()',
    'OpenGlBackend::verifyRuntimeSupport()',
    'qCInfo(playerGraphicsLog)',
    'qCCritical(playerGraphicsLog)'
)) {
    if (-not $graphicsBackendSource.Contains($fragment)) {
        throw "GraphicsBackend source is missing required backend validation fragment: $fragment"
    }
}

$openGlBackendSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/graphics_backend/opengl_backend.cpp") -Raw
foreach ($fragment in @(
    'QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL)',
    'QSurfaceFormat::OpenGL',
    'QOffscreenSurface',
    'QOpenGLContext',
    'QOpenGLFunctions',
    'glGetString(GL_VERSION)',
    'supportsOpenGL()',
    'supportsOpenGles()',
    'QOpenGLContext::openGLModuleType()'
)) {
    if (-not $openGlBackendSource.Contains($fragment)) {
        throw "OpenGlBackend source is missing required OpenGL validation fragment: $fragment"
    }
}

$applicationContainerSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/composition/application_container.cpp") -Raw
foreach ($fragment in @(
    'ApplicationContainer::initialize()',
    'ApplicationContainer::shutdown()',
    'qCInfo(playerApplicationLog)',
    'qCCritical(playerApplicationLog)'
)) {
    if (-not $applicationContainerSource.Contains($fragment)) {
        throw "ApplicationContainer is missing required lifecycle fragment: $fragment"
    }
}

$qmlBootstrapSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/qml_bootstrap.cpp") -Raw
foreach ($fragment in @(
    'QQmlApplicationEngine',
    'loadFromModule',
    'Player',
    'App',
    'objectCreationFailed'
)) {
    if (-not $qmlBootstrapSource.Contains($fragment)) {
        throw "QmlBootstrap source is missing required QML loading fragment: $fragment"
    }
}

$qmlDir = Get-Content -LiteralPath (Join-Path $projectRoot "src/presentation/qml/qmldir") -Raw
foreach ($fragment in @(
    'module Player',
    'App 1.0 App.qml',
    'MainWindow 1.0 MainWindow.qml'
)) {
    if (-not $qmlDir.Contains($fragment)) {
        throw "QML module manifest is missing required fragment: $fragment"
    }
}

$playerScreenQml = Get-Content -LiteralPath (Join-Path $projectRoot "src/presentation/qml/screens/player/PlayerScreen.qml") -Raw
foreach ($fragment in @(
    "VideoSurface",
    "PlayerChrome"
)) {
    if (-not $playerScreenQml.Contains($fragment)) {
        throw "PlayerScreen.qml must remain a composition-only player shell: $fragment"
    }
}
if ($playerScreenQml -match '(?i)mpv_command|mpv_set_property|mpv_get_property') {
    throw "PlayerScreen.qml must not access libmpv directly."
}

$testsCMake = Get-Content -LiteralPath (Join-Path $projectRoot "tests/CMakeLists.txt") -Raw
foreach ($fragment in @(
    "application_container_tests",
    "NAME application_container",
    "mpv_runtime_probe_tests",
    "NAME mpv_runtime_probe",
    "player_stage_libmpv_runtime(mpv_runtime_probe_tests)"
)) {
    if (-not $testsCMake.Contains($fragment)) {
        throw "tests/CMakeLists.txt is missing required R1/R2 test target behavior: $fragment"
    }
}

$configureScript = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/configure.ps1") -Raw
if (-not $configureScript.Contains('"--fresh", "--preset"')) {
    throw "configure.ps1 must use CMake --fresh so generated cache paths cannot bind a moved or renamed checkout to its previous location."
}

$developmentRuntimeModule = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/modules/DevelopmentRuntime.psm1") -Raw
foreach ($fragment in @(
    'function Assert-PlayerDevelopmentRuntimeMarker',
    'function Set-PlayerDevelopmentRuntimeMarker',
    '$markerDirectory = Join-Path $outputDirectory "cmake"',
    '.player-development-root',
    '"../../.."',
    '[System.IO.File]::WriteAllText',
    '[System.IO.Path]::IsPathRooted',
    '[OK] Development runtime root marker'
)) {
    if (-not $developmentRuntimeModule.Contains($fragment)) {
        throw "DevelopmentRuntime.psm1 is missing required marker staging/verification fragment: $fragment"
    }
}
if ($developmentRuntimeModule -match '[A-Za-z]:[/\\]') {
    throw "DevelopmentRuntime.psm1 contains a machine-absolute Windows path."
}

$buildScript = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/build.ps1") -Raw
foreach ($fragment in @(
    'modules/DevelopmentRuntime.psm1',
    'modules/QtRuntimeDeployment.psm1',
    'Resolve-PlayerQtRoot -Layout $layout',
    'Set-PlayerDevelopmentRuntimeMarker',
    'Invoke-PlayerQtRuntimeDeployment',
    '-Preset $Preset',
    '-ProjectRoot $projectRoot',
    '-QtRoot $qtRoot'
)) {
    if (-not $buildScript.Contains($fragment)) {
        throw "build.ps1 is missing required post-build runtime staging fragment: $fragment"
    }
}

$testScript = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/test.ps1") -Raw
foreach ($fragment in @(
    'modules/DevelopmentRuntime.psm1',
    'Assert-PlayerDevelopmentRuntimeMarker',
    'Resolve-PlayerQtRoot -Layout $layout',
    'QT_PLUGIN_PATH',
    'QT_QPA_PLATFORM_PLUGIN_PATH',
    'cmake --build --preset $Preset',
    'ctest --preset $Preset'
)) {
    if (-not $testScript.Contains($fragment)) {
        throw "test.ps1 is missing required test runtime fragment: $fragment"
    }
}

$launchScript = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/launch.ps1") -Raw
foreach ($fragment in @(
    'modules/DevelopmentRuntime.psm1',
    'Assert-PlayerDevelopmentRuntimeMarker',
    'Resolve-PlayerQtRoot -Layout $layout',
    'Player.exe',
    'QT_PLUGIN_PATH',
    'QT_QPA_PLATFORM_PLUGIN_PATH'
)) {
    if (-not $launchScript.Contains($fragment)) {
        throw "launch.ps1 is missing required launch runtime fragment: $fragment"
    }
}

$qtRuntimeModule = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/modules/QtRuntimeDeployment.psm1") -Raw
foreach ($fragment in @(
    'function Invoke-PlayerQtRuntimeDeployment',
    'Resolve-PlayerWindeployQt',
    '--dir',
    '--debug',
    '--release',
    '--no-translations'
)) {
    if (-not $qtRuntimeModule.Contains($fragment)) {
        throw "QtRuntimeDeployment.psm1 is missing required deployment fragment: $fragment"
    }
}

$mpvRootCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'add_library(player_mpv_infrastructure STATIC)',
    'add_subdirectory(client)',
    'add_subdirectory(initialization)',
    'add_subdirectory(errors)',
    'add_subdirectory(commands)',
    'add_subdirectory(events)',
    'add_subdirectory(properties)',
    'add_subdirectory(runtime)',
    'player_apply_compiler_options(player_mpv_infrastructure)',
    'player_enable_compiler_warnings(player_mpv_infrastructure)',
    'player_enable_sanitizers(player_mpv_infrastructure)',
    'player_enable_static_analysis(player_mpv_infrastructure)',
    'LibMpv::LibMpv',
    'PLAYER_LIBMPV_RUNTIME_FILENAME',
    'PLAYER_EXPECTED_MPV_VERSION',
    'PLAYER_EXPECTED_MPV_TAG',
    'PLAYER_EXPECTED_MPV_COMMIT',
    'PLAYER_EXPECTED_FFMPEG_VERSION'
)) {
    if (-not $mpvRootCMake.Contains($fragment)) {
        throw "src/playback/infrastructure/mpv/CMakeLists.txt is missing required R2 infrastructure fragment: $fragment"
    }
}

$mpvClientCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/client/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'mpv_handle.cpp',
    'mpv_handle.h'
)) {
    if (-not $mpvClientCMake.Contains($fragment)) {
        throw "mpv client CMake is missing required handle fragment: $fragment"
    }
}

$mpvInitializationCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/initialization/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'mpv_initializer.cpp',
    'mpv_initializer.h',
    'mpv_option_profile.cpp',
    'mpv_option_profile.h'
)) {
    if (-not $mpvInitializationCMake.Contains($fragment)) {
        throw "mpv initialization CMake is missing required module fragment: $fragment"
    }
}

$mpvEventsCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/events/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'mpv_event.h',
    'mpv_event_decoder.cpp',
    'mpv_event_decoder.h',
    'mpv_event_loop.cpp',
    'mpv_event_loop.h',
    'mpv_wakeup_bridge.cpp',
    'mpv_wakeup_bridge.h'
)) {
    if (-not $mpvEventsCMake.Contains($fragment)) {
        throw "mpv events CMake is missing required event fragment: $fragment"
    }
}

$mpvCommandsCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/commands/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'mpv_command_encoder.cpp',
    'mpv_command_encoder.h',
    'mpv_command_executor.cpp',
    'mpv_command_executor.h',
    'mpv_command_request.h'
)) {
    if (-not $mpvCommandsCMake.Contains($fragment)) {
        throw "mpv commands CMake is missing required command fragment: $fragment"
    }
}

$mpvPropertiesCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/properties/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'mpv_node_decoder.cpp',
    'mpv_node_decoder.h',
    'mpv_property_change.h',
    'mpv_property_observer.cpp',
    'mpv_property_observer.h',
    'mpv_property_registry.cpp',
    'mpv_property_registry.h'
)) {
    if (-not $mpvPropertiesCMake.Contains($fragment)) {
        throw "mpv properties CMake is missing required property fragment: $fragment"
    }
}

$mpvRuntimeCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/runtime/CMakeLists.txt") -Raw
foreach ($fragment in @(
    'mpv_runtime_probe.cpp',
    'mpv_runtime_probe.h'
)) {
    if (-not $mpvRuntimeCMake.Contains($fragment)) {
        throw "mpv runtime CMake is missing required runtime fragment: $fragment"
    }
}

$mpvHandleHeader = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/client/mpv_handle.h") -Raw
foreach ($fragment in @(
    'class MpvHandle final',
    'MpvHandle(const MpvHandle&) = delete',
    'MpvHandle& operator=(const MpvHandle&) = delete',
    'mpv_handle* get() const noexcept',
    'void close() noexcept'
)) {
    if (-not $mpvHandleHeader.Contains($fragment)) {
        throw "MpvHandle header is missing required ownership fragment: $fragment"
    }
}

$mpvHandleSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/playback/infrastructure/mpv/client/mpv_handle.cpp") -Raw
foreach ($fragment in @(
    'mpv_create()',
    'mpv_initialize',
    'mpv_destroy',
    'mpv_terminate_destroy'
)) {
    if (-not $mpvHandleSource.Contains($fragment)) {
        throw "MpvHandle source is missing required libmpv lifecycle fragment: $fragment"
    }
}

$mainCpp = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/main.cpp") -Raw
if ($mainCpp -match '(?i)mpv_command|mpv_set_property|mpv_get_property') {
    throw "src/app/main.cpp must not access libmpv directly."
}

$mainWindowQml = Get-Content -LiteralPath (Join-Path $projectRoot "src/presentation/qml/MainWindow.qml") -Raw
if ($mainWindowQml -match '(?i)mpv_command|mpv_set_property|mpv_get_property') {
    throw "MainWindow.qml must not access libmpv directly."
}

Write-Host "Project layout verification passed."
