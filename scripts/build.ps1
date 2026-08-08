param(
    [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
    [string]$Preset = "windows-msvc-debug"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
$developmentRuntimeModulePath = Join-Path $PSScriptRoot "modules/DevelopmentRuntime.psm1"
$qtRuntimeModulePath = Join-Path $PSScriptRoot "modules/QtRuntimeDeployment.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force
Import-Module $developmentRuntimeModulePath -Force
Import-Module $qtRuntimeModulePath -Force

Push-Location $projectRoot
try {
    & (Join-Path $PSScriptRoot "verify-dependencies.ps1")

    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $cmake = Resolve-PlayerCMake -Layout $layout
    $qtRoot = Resolve-PlayerQtRoot -Layout $layout

    & $cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed with exit code $LASTEXITCODE."
    }

    Assert-PlayerDevelopmentRuntimeMarker `
        -Preset $Preset `
        -ProjectRoot $projectRoot

    Invoke-PlayerQtRuntimeDeployment `
        -Preset $Preset `
        -ProjectRoot $projectRoot `
        -QtRoot $qtRoot
}
finally {
    Pop-Location
}
