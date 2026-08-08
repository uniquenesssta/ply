param(
    [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
    [string]$Preset = "windows-msvc-debug"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
$msvcModulePath = Join-Path $PSScriptRoot "modules/MsvcEnvironment.psm1"
$qtRuntimeModulePath = Join-Path $PSScriptRoot "modules/QtRuntimeDeployment.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force
Import-Module $msvcModulePath -Force
Import-Module $qtRuntimeModulePath -Force

Push-Location $projectRoot
try {
    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $qtRoot = Resolve-PlayerQtRoot -Layout $layout

    Initialize-PlayerMsvcEnvironment -Versions $versions

    Invoke-PlayerQtRuntimeDeployment `
        -Preset $Preset `
        -ProjectRoot $projectRoot `
        -QtRoot $qtRoot
}
finally {
    Pop-Location
}
