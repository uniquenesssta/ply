$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot

$requiredFiles = @(
    "CMakeLists.txt",
    "CMakePresets.json",
    "cmake/CMakeLists.txt",
    "cmake/AppTargets.cmake",
    "cmake/CompilerOptions.cmake",
    "cmake/CompilerWarnings.cmake",
    "cmake/DependencyPaths.cmake",
    "cmake/DependencyVersions.cmake",
    "cmake/Sanitizers.cmake",
    "cmake/StaticAnalysis.cmake",
    "scripts/configure.ps1",
    "scripts/test.ps1",
    "scripts/modules/DependencyPaths.psm1",
    "scripts/modules/DependencyVersions.psm1",
    "scripts/modules/MsvcEnvironment.psm1",
    "src/CMakeLists.txt",
    "src/app/CMakeLists.txt",
    "src/app/main.cpp",
    "src/app/bootstrap/application_bootstrap.cpp",
    "src/app/bootstrap/application_bootstrap.h",
    "src/app/bootstrap/graphics_backend/CMakeLists.txt",
    "src/app/bootstrap/graphics_backend/graphics_backend_bootstrap.cpp",
    "src/app/bootstrap/graphics_backend/graphics_backend_bootstrap.h",
    "src/app/bootstrap/graphics_backend/graphics_backend_probe.cpp",
    "src/app/bootstrap/graphics_backend/graphics_backend_probe.h",
    "src/app/bootstrap/logging_bootstrap.cpp",
    "src/app/bootstrap/logging_bootstrap.h",
    "src/app/bootstrap/qml_bootstrap.cpp",
    "src/app/bootstrap/qml_bootstrap.h",
    "src/app/bootstrap/runtime_paths.cpp",
    "src/app/bootstrap/runtime_paths.h",
    "src/app/composition/CMakeLists.txt",
    "src/app/composition/application_container.cpp",
    "src/app/composition/application_container.h",
    "src/foundation/CMakeLists.txt",
    "src/foundation/logging/CMakeLists.txt",
    "src/foundation/logging/log_categories.cpp",
    "src/foundation/logging/log_categories.h",
    "src/foundation/logging/log_file_sink.cpp",
    "src/foundation/logging/log_file_sink.h",
    "src/foundation/logging/log_redactor.cpp",
    "src/foundation/logging/log_redactor.h",
    "src/presentation/CMakeLists.txt",
    "src/presentation/qml/App.qml",
    "src/presentation/qml/shell/MainWindow.qml",
    "src/presentation/qml/screens/player/PlayerScreen.qml",
    "src/presentation/qml/features/player/video/VideoSurface.qml",
    "src/presentation/qml/features/player/chrome/PlayerChrome.qml",
    "src/presentation/qml/theme/Theme.qml",
    "tests/CMakeLists.txt",
    "tests/unit/app/bootstrap/graphics_backend/graphics_backend_probe_test.cpp",
    "tests/unit/app/bootstrap/runtime_paths_test.cpp",
    "tests/unit/app/composition/application_container_test.cpp",
    "tests/unit/foundation/logging/logging_test.cpp"
)

$missingFiles = [System.Collections.Generic.List[string]]::new()
foreach ($relativePath in $requiredFiles) {
    $fullPath = Join-Path $projectRoot $relativePath
    if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
        [void]$missingFiles.Add($relativePath)
    }
}

if ($missingFiles.Count -gt 0) {
    $missingList = ($missingFiles | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
    throw @"
Project layout verification failed. Missing files:
$missingList

Restore the complete scaffold before configuring.
"@
}

foreach ($obsoletePath in @(
    "src/app/bootstrap/graphics_backend_bootstrap.cpp",
    "src/app/bootstrap/graphics_backend_bootstrap.h"
)) {
    if (Test-Path -LiteralPath (Join-Path $projectRoot $obsoletePath)) {
        throw "Obsolete pre-R1-04 graphics backend file remains in the source tree: $obsoletePath"
    }
}

$topLevelCMake = Get-Content -LiteralPath (Join-Path $projectRoot "CMakeLists.txt") -Raw

foreach ($fragment in @(
    "option(PLAYER_BUILD_TESTS",
    "include(CTest)",
    "add_subdirectory(cmake)"
)) {
    if (-not $topLevelCMake.Contains($fragment)) {
        throw "Top-level CMake orchestration is missing required fragment: $fragment"
    }
}

foreach ($fragment in @(
    "find_package(",
    "qt_standard_project_setup(",
    "qt_add_executable(",
    "add_executable(",
    "add_library(",
    "target_sources(",
    "target_link_libraries(",
    "install(",
    "add_subdirectory(src)",
    "add_subdirectory(tests)"
)) {
    if ($topLevelCMake.Contains($fragment)) {
        throw "Top-level CMake contains responsibility that must remain below cmake/: $fragment"
    }
}

$orchestrator = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/CMakeLists.txt") -Raw

foreach ($fragment in @(
    "include(DependencyVersions)",
    "include(DependencyPaths)",
    "include(AppTargets)",
    'Qt6 ${PLAYER_QT_VERSION} EXACT',
    'add_subdirectory("../src" "../src")',
    'add_subdirectory("../tests" "../tests")'
)) {
    if (-not $orchestrator.Contains($fragment)) {
        throw "cmake/CMakeLists.txt is missing required orchestration fragment: $fragment"
    }
}

$appCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/CMakeLists.txt") -Raw
foreach ($fragment in @(
    "add_subdirectory(bootstrap/graphics_backend)",
    "add_subdirectory(composition)"
)) {
    if (-not $appCMake.Contains($fragment)) {
        throw "src/app/CMakeLists.txt is missing required app module delegation: $fragment"
    }
}

$applicationBootstrap = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/application_bootstrap.cpp") -Raw
foreach ($fragment in @(
    "ApplicationContainer container",
    "container.loggingBootstrap()",
    "container.qmlBootstrap()",
    "qmlBootstrap.lastError()",
    "container.shutdown()"
)) {
    if (-not $applicationBootstrap.Contains($fragment)) {
        throw "ApplicationBootstrap is missing required composition/QML bootstrap usage: $fragment"
    }
}
foreach ($forbiddenFragment in @(
    "LoggingBootstrap loggingBootstrap;",
    "QmlBootstrap qmlBootstrap;"
)) {
    if ($applicationBootstrap.Contains($forbiddenFragment)) {
        throw "ApplicationBootstrap reintroduced top-level object ownership outside ApplicationContainer: $forbiddenFragment"
    }
}

$applicationContainerHeader = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/composition/application_container.h") -Raw
foreach ($fragment in @(
    "std::unique_ptr<LoggingBootstrap>",
    "std::unique_ptr<QmlBootstrap>",
    "void shutdown() noexcept",
    "ApplicationContainer(const ApplicationContainer&) = delete"
)) {
    if (-not $applicationContainerHeader.Contains($fragment)) {
        throw "ApplicationContainer is missing required ownership/lifecycle fragment: $fragment"
    }
}

$applicationContainerSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/composition/application_container.cpp") -Raw
$qmlShutdownIndex = $applicationContainerSource.IndexOf("qmlBootstrap_.reset();", [System.StringComparison]::Ordinal)
$loggingShutdownIndex = $applicationContainerSource.IndexOf("loggingBootstrap_->stop();", [System.StringComparison]::Ordinal)
if ($qmlShutdownIndex -lt 0 -or $loggingShutdownIndex -lt 0 -or $qmlShutdownIndex -gt $loggingShutdownIndex) {
    throw "ApplicationContainer shutdown must destroy QML ownership before stopping logging."
}

$qmlBootstrapHeader = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/qml_bootstrap.h") -Raw
foreach ($fragment in @(
    "QmlBootstrap();",
    "const QString& lastError() const noexcept",
    "QStringList warningMessages_",
    "QString lastError_"
)) {
    if (-not $qmlBootstrapHeader.Contains($fragment)) {
        throw "QmlBootstrap is missing required R1-06 diagnostic state: $fragment"
    }
}

$qmlBootstrapSource = Get-Content -LiteralPath (Join-Path $projectRoot "src/app/bootstrap/qml_bootstrap.cpp") -Raw
foreach ($fragment in @(
    "QQmlEngine::warnings",
    "warning.toString()",
    'loadFromModule("Player.Presentation", "App")',
    "engine_.rootObjects().isEmpty()",
    "QML warning:"
)) {
    if (-not $qmlBootstrapSource.Contains($fragment)) {
        throw "QmlBootstrap is missing required R1-06 load diagnostic behavior: $fragment"
    }
}

$presentationCMake = Get-Content -LiteralPath (Join-Path $projectRoot "src/presentation/CMakeLists.txt") -Raw
foreach ($fragment in @(
    "qml/App.qml",
    "qml/shell/MainWindow.qml",
    "qml/screens/player/PlayerScreen.qml",
    "qt_add_qml_module(",
    "URI Player.Presentation"
)) {
    if (-not $presentationCMake.Contains($fragment)) {
        throw "Presentation CMake is missing required R1-06 QML shell/module fragment: $fragment"
    }
}

$appQml = Get-Content -LiteralPath (Join-Path $projectRoot "src/presentation/qml/App.qml") -Raw
if (-not $appQml.Contains("MainWindow {}")) {
    throw "App.qml must remain a root-shell composition entry and instantiate MainWindow."
}

$mainWindowQml = Get-Content -LiteralPath (Join-Path $projectRoot "src/presentation/qml/shell/MainWindow.qml") -Raw
foreach ($fragment in @(
    "ApplicationWindow",
    "visible: true",
    "PlayerScreen"
)) {
    if (-not $mainWindowQml.Contains($fragment)) {
        throw "MainWindow.qml is missing required R1-06 shell behavior: $fragment"
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
    "NAME application_container"
)) {
    if (-not $testsCMake.Contains($fragment)) {
        throw "tests/CMakeLists.txt is missing the R1-05 ApplicationContainer test target: $fragment"
    }
}

$configureScript = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/configure.ps1") -Raw
if (-not $configureScript.Contains('"--fresh", "--preset"')) {
    throw "configure.ps1 must use CMake --fresh so generated cache paths cannot bind a moved or renamed checkout to its previous location."
}

$testScript = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/test.ps1") -Raw
foreach ($fragment in @(
    "Resolve-PlayerCTest",
    '& $ctest --preset $Preset',
    'Join-Path $qtRoot "bin"',
    'Join-Path $qtRoot "plugins"',
    'QT_PLUGIN_PATH',
    'QT_QPA_PLATFORM_PLUGIN_PATH'
)) {
    if (-not $testScript.Contains($fragment)) {
        throw "test.ps1 is missing required test-runtime fragment: $fragment"
    }
}
if ($testScript.Contains('& $cmake --test')) {
    throw "test.ps1 must not call the unsupported 'cmake --test' form."
}
if ($testScript -match '[A-Za-z]:[/\\]') {
    throw "test.ps1 contains a machine-absolute Windows path; Qt test runtime paths must be derived from the parent-relative Qt root."
}

$dependencyPaths = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/DependencyPaths.cmake") -Raw
if ($dependencyPaths -match '[A-Za-z]:[/\\]') {
    throw "cmake/DependencyPaths.cmake contains a machine-absolute Windows path."
}

$pathModule = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/modules/DependencyPaths.psm1") -Raw
if ($pathModule -match '[A-Za-z]:[/\\]') {
    throw "DependencyPaths.psm1 contains a machine-absolute Windows path."
}
if ($pathModule -match '(?i)\.\.[/\\]cmake[/\\]' -or $pathModule -match '(?i)\.\.[/\\]ninja[/\\]') {
    throw "DependencyPaths.psm1 reintroduced an unsupported parent-level ../cmake or ../ninja dependency root."
}
foreach ($fragment in @(
    "../Qt/",
    "../libmpv/",
    "../downloads",
    "../cache/",
    "QtToolsRootRelative",
    "CMake_64/bin/cmake.exe",
    "CMake_64/bin/ctest.exe",
    "Ninja/ninja.exe",
    "Resolve-PlayerCTest",
    "-replace '\\', '/'"
)) {
    if (-not $pathModule.Contains($fragment)) {
        throw "DependencyPaths.psm1 is missing required parent-workspace fragment: $fragment"
    }
}

$msvcEnvironment = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/modules/MsvcEnvironment.psm1") -Raw
foreach ($fragment in @(
    "vswhere.exe",
    "vcvars64.bat",
    "VSCMD_ARG_TGT_ARCH",
    "SetEnvironmentVariable"
)) {
    if (-not $msvcEnvironment.Contains($fragment)) {
        throw "MsvcEnvironment.psm1 is missing required Visual Studio environment fragment: $fragment"
    }
}

$dependencyVerifier = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/verify-dependencies.ps1") -Raw
foreach ($fragment in @(
    "Test-PlayerMinimumToolVersion",
    "FileVersionInfo",
    "VCToolsVersion",
    "Compatible development tools"
)) {
    if (-not $dependencyVerifier.Contains($fragment)) {
        throw "verify-dependencies.ps1 is missing required compatibility-verification fragment: $fragment"
    }
}

Write-Host "Project layout, R1-06 QML shell diagnostics, R1-05 composition root ownership, R1-04 graphics backend module, Qt test runtime environment, CTest preset entry, fresh relocatable configure policy, parent-workspace relative paths, Windows path normalization, compatible tool gates, and CMake responsibility boundaries are complete."
