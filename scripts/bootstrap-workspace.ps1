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

    Write-Host "Repository-relative dependency layout is ready:"
    Write-Host "  Qt:           $($layout.QtRootRelative)"
    Write-Host "  libmpv:       $($layout.LibMpvRootRelative)"
    Write-Host "  Downloads:    $($layout.DownloadsRootRelative)"
    Write-Host "  FetchContent: $($layout.FetchContentRootRelative)"
    Write-Host ""
    Write-Host "CMake/Ninja are resolved from PATH; optional portable copies may live at:"
    Write-Host "  CMake:        $($layout.PortableCMakeRelative)"
    Write-Host "  Ninja:        $($layout.PortableNinjaRelative)"
}
finally {
    Pop-Location
}
