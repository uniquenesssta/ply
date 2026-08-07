param(
    [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
    [string]$Preset = "windows-msvc-debug"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force

Push-Location $projectRoot
try {
    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $cmake = Resolve-PlayerCMake -Layout $layout

    & $cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
