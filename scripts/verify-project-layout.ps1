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
    "src/app/bootstrap/runtime_paths.cpp",
    "src/app/bootstrap/runtime_paths.h",
    "src/presentation/CMakeLists.txt",
    "src/presentation/qml/App.qml",
    "tests/CMakeLists.txt",
    "tests/unit/app/bootstrap/runtime_paths_test.cpp"
)

$missingFiles = [System.Collections.Generic.List[string]]::new()
foreach ($relativePath in $requiredFiles) {
    $fullPath = Join-Path $projectRoot $relativePath
    if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
        $missingFiles.Add($relativePath)
    }
}

if ($missingFiles.Count -gt 0) {
    $missingList = ($missingFiles | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
    throw @"
Project layout verification failed. Missing files:
$missingList

This usually means a small update package was extracted as a standalone project.
Restore the complete scaffold before configuring.
"@
}

$topLevelCMakePath = Join-Path $projectRoot "CMakeLists.txt"
$topLevelCMake = Get-Content -LiteralPath $topLevelCMakePath -Raw

$requiredTopLevelFragments = @(
    "option(PLAYER_BUILD_TESTS",
    "include(CTest)",
    "add_subdirectory(cmake)"
)

foreach ($fragment in $requiredTopLevelFragments) {
    if (-not $topLevelCMake.Contains($fragment)) {
        throw "Top-level CMake orchestration is missing required fragment: $fragment"
    }
}

$forbiddenTopLevelFragments = @(
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
)

foreach ($fragment in $forbiddenTopLevelFragments) {
    if ($topLevelCMake.Contains($fragment)) {
        throw "Top-level CMake contains responsibility that must remain below cmake/: $fragment"
    }
}

$orchestratorPath = Join-Path $projectRoot "cmake/CMakeLists.txt"
$orchestrator = Get-Content -LiteralPath $orchestratorPath -Raw

$requiredOrchestratorFragments = @(
    "include(DependencyVersions)",
    "include(DependencyPaths)",
    "include(AppTargets)",
    'Qt6 ${PLAYER_QT_VERSION} EXACT',
    'add_subdirectory("${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/src")',
    'add_subdirectory("${PROJECT_SOURCE_DIR}/tests" "${PROJECT_BINARY_DIR}/tests")'
)

foreach ($fragment in $requiredOrchestratorFragments) {
    if (-not $orchestrator.Contains($fragment)) {
        throw "cmake/CMakeLists.txt is missing required orchestration fragment: $fragment"
    }
}

Write-Host "Project layout and CMake responsibility boundaries are complete."
