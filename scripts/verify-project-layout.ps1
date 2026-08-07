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
    "src/app/bootstrap/runtime_paths.cpp",
    "src/app/bootstrap/runtime_paths.h",
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
    "tests/CMakeLists.txt",
    "tests/unit/app/bootstrap/graphics_backend/graphics_backend_probe_test.cpp",
    "tests/unit/app/bootstrap/runtime_paths_test.cpp",
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
if (-not $appCMake.Contains("add_subdirectory(bootstrap/graphics_backend)")) {
    throw "src/app/CMakeLists.txt must delegate the R1-04 graphics backend module to bootstrap/graphics_backend/."
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
    "Ninja/ninja.exe",
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

Write-Host "Project layout, R1-04 graphics backend module, parent-workspace relative paths, Windows path normalization, compatible tool gates, and CMake responsibility boundaries are complete."
