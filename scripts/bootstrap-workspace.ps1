$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force

Push-Location $projectRoot
try {
    $versions = Get-PlayerDependencyVersions -ProjectRoot $projectRoot
    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot -Versions $versions
    Initialize-PlayerDependencyLayout -Layout $layout

    Write-Host "Pinned repository-parent dependency layout is ready:"
    Write-Host "  Qt:           $($layout.QtRootRelative)"
    Write-Host "  libmpv:       $($layout.LibMpvRootRelative)"
    Write-Host "  CMake:        $($layout.CMakeRootRelative)"
    Write-Host "  Ninja:        $($layout.NinjaRootRelative)"
    Write-Host "  Downloads:    $($layout.DownloadsRootRelative)"
    Write-Host "  FetchContent: $($layout.FetchContentRootRelative)"
    Write-Host ""
    Write-Host "Qt, CMake, and Ninja installation directories are not created or modified by this script."
}
finally {
    Pop-Location
}
