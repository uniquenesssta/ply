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
    Initialize-PlayerDependencyLayout -Layout $layout

    Write-Host "Repository-parent workspace layout is ready:"
    Write-Host "  Qt:           $($layout.QtRootRelative)"
    Write-Host "  Qt tools:     $($layout.QtToolsRootRelative)"
    Write-Host "  libmpv:       $($layout.LibMpvRootRelative)"
    Write-Host "  Downloads:    $($layout.DownloadsRootRelative)"
    Write-Host "  FetchContent: $($layout.FetchContentRootRelative)"
    Write-Host ""
    Write-Host "CMake/Ninja are development tools, not extra parent-level dependency folders."
    Write-Host "They are resolved from PATH first, then from the existing Qt tools tree:"
    Write-Host "  CMake:        ../Qt/Tools/CMake_64/bin/cmake.exe"
    Write-Host "  Ninja:        ../Qt/Tools/Ninja/ninja.exe"
}
finally {
    Pop-Location
}
