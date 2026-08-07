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
    "src/CMakeLists.txt",
    "src/app/CMakeLists.txt",
    "src/app/main.cpp",
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

$dependencyPaths = Get-Content -LiteralPath (Join-Path $projectRoot "cmake/DependencyPaths.cmake") -Raw
$pathModule = Get-Content -LiteralPath (Join-Path $projectRoot "scripts/modules/DependencyPaths.psm1") -Raw

foreach ($contentCheck in @(
    @{ Name = "cmake/DependencyPaths.cmake"; Content = $dependencyPaths },
    @{ Name = "scripts/modules/DependencyPaths.psm1"; Content = $pathModule }
)) {
    if ($contentCheck.Content -match '[A-Za-z]:[/\\]') {
        throw "$($contentCheck.Name) contains a machine-absolute Windows path."
    }

    if ($contentCheck.Content -match '(?i)\.\./cmake/' -or $contentCheck.Content -match '(?i)\.\./ninja/') {
        throw "$($contentCheck.Name) invents a parent-level cmake/ninja directory. The parent workspace is limited to Qt, libmpv, downloads, and cache."
    }
}

foreach ($requiredPathFragment in @(
    '../Qt/',
    '../libmpv/',
    '../downloads',
    '../cache/'
)) {
    if (-not $dependencyPaths.Contains($requiredPathFragment)) {
        throw "cmake/DependencyPaths.cmake is missing required repository-parent path: $requiredPathFragment"
    }
}

if (-not $pathModule.Contains('../Qt/Tools')) {
    throw "DependencyPaths.psm1 must resolve development tools from the existing ../Qt/Tools tree when PATH does not provide them."
}

Write-Host "Project layout, parent-workspace relative paths, and CMake responsibility boundaries are complete."
