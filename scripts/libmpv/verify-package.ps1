$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$versionModulePath = Join-Path $projectRoot "scripts/modules/DependencyVersions.psm1"
$dependencyPathModulePath = Join-Path $projectRoot "scripts/modules/DependencyPaths.psm1"
$buildPathModulePath = Join-Path $PSScriptRoot "modules/LibMpvBuildPaths.psm1"
$packageModulePath = Join-Path $PSScriptRoot "modules/LibMpvPackage.psm1"

Import-Module $versionModulePath -Force
Import-Module $dependencyPathModulePath -Force
Import-Module $buildPathModulePath -Force
Import-Module $packageModulePath -Force

Push-Location $projectRoot
try {
    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $workspaceLayout = Get-PlayerWorkspaceLayout -Versions $versions
    $buildLayout = Get-PlayerLibMpvBuildLayout -Versions $versions

    $package = Resolve-PlayerLibMpvPackage -Layout $workspaceLayout
    $manifest = Test-PlayerLibMpvDependencyManifest -Versions $versions -Layout $buildLayout

    Write-Host "[OK] libmpv header: $($package.HeaderRelative)"
    Write-Host "[OK] libmpv import library: $($package.ImportLibraryRelative)"
    Write-Host "[OK] libmpv runtime DLL: $($package.RuntimeLibraryRelative)"
    Write-Host "[OK] manifest identity: mpv $($manifest.mpv.version), FFmpeg $($manifest.ffmpeg.version), zlib $($manifest.zlib.version), libplacebo $($manifest.libplacebo.version), libass $($manifest.libass.version)"
    Write-Host "[OK] manifest artifact hashes: $(@($manifest.artifacts).Count) verified"
    Write-Host "libmpv package verification passed."
}
finally {
    Pop-Location
}
