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
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force
Import-Module $developmentRuntimeModulePath -Force

Push-Location $projectRoot
try {
    & (Join-Path $PSScriptRoot "verify-project-layout.ps1")
    & (Join-Path $PSScriptRoot "verify-dependencies.ps1")

    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $cmake = Resolve-PlayerCMake -Layout $layout

    $ninja = Resolve-PlayerNinja -Layout $layout
    Resolve-PlayerQtRoot -Layout $layout | Out-Null

    # Any fresh configure invalidates the previous development-build acceptance
    # marker. Only a later successful build may create it again.
    Clear-PlayerDevelopmentRuntimeMarker `
        -Preset $Preset `
        -ProjectRoot $projectRoot

    # CMakeCache.txt stores resolved absolute source/build paths by design. Always
    # regenerate it so a moved, renamed, or copied checkout is not tied to the
    # previous repository location.
    $configureArguments = @("--fresh", "--preset", $Preset)
    if ($ninja -match '[/\\]') {
        $configureArguments += "-DCMAKE_MAKE_PROGRAM:FILEPATH=$ninja"
    }

    & $cmake @configureArguments
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configure failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}