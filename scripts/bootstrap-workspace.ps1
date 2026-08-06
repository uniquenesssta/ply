$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$modulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $modulePath -Force

Push-Location $projectRoot
try {
    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot
    Initialize-PlayerDependencyLayout -Layout $layout

    Write-Host "Repository-parent dependency layout is ready:"
    Write-Host "  Parent:       .."
    Write-Host "  Qt:           ../Qt/<version>/msvc2022_64"
    Write-Host "  libmpv:       ../libmpv/windows-x64"
    Write-Host "  CMake:        ../cmake"
    Write-Host "  Ninja:        ../ninja"
    Write-Host "  Downloads:    ../downloads"
    Write-Host "  Cache:        ../cache"
}
finally {
    Pop-Location
}
