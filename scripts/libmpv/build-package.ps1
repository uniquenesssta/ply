$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$versionModulePath = Join-Path $projectRoot "scripts/modules/DependencyVersions.psm1"
$msvcModulePath = Join-Path $projectRoot "scripts/modules/MsvcEnvironment.psm1"
$msysModulePath = Join-Path $PSScriptRoot "modules/Msys2Environment.psm1"
$buildPathModulePath = Join-Path $PSScriptRoot "modules/LibMpvBuildPaths.psm1"
$packageModulePath = Join-Path $PSScriptRoot "modules/LibMpvPackage.psm1"

Import-Module $versionModulePath -Force
Import-Module $msvcModulePath -Force
Import-Module $msysModulePath -Force
Import-Module $buildPathModulePath -Force
Import-Module $packageModulePath -Force

Push-Location $projectRoot
$previousEnvironment = @{}
try {
    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerLibMpvBuildLayout -Versions $versions
    Initialize-PlayerLibMpvBuildLayout -Layout $layout

    $environmentValues = [ordered]@{
        PLAYER_MPV_VERSION        = $versions.MpvVersion
        PLAYER_MPV_TAG            = $versions.MpvTag
        PLAYER_MPV_COMMIT         = $versions.MpvCommit
        PLAYER_FFMPEG_VERSION     = $versions.FfmpegVersion
        PLAYER_FFMPEG_REF         = $versions.FfmpegRef
        PLAYER_FFMPEG_COMMIT      = $versions.FfmpegCommit
        PLAYER_LIBPLACEBO_VERSION = $versions.LibplaceboVersion
        PLAYER_LIBPLACEBO_REF     = $versions.LibplaceboRef
        PLAYER_LIBPLACEBO_COMMIT  = $versions.LibplaceboCommit
        PLAYER_LIBASS_VERSION     = $versions.LibassVersion
        PLAYER_LIBASS_REF         = $versions.LibassRef
        PLAYER_LIBASS_COMMIT      = $versions.LibassCommit
        PLAYER_FREETYPE_VERSION   = $versions.FreetypeVersion
        PLAYER_FREETYPE_REF       = $versions.FreetypeRef
        PLAYER_FREETYPE_COMMIT    = $versions.FreetypeCommit
        PLAYER_FRIBIDI_VERSION    = $versions.FribidiVersion
        PLAYER_FRIBIDI_REF        = $versions.FribidiRef
        PLAYER_FRIBIDI_COMMIT     = $versions.FribidiCommit
        PLAYER_HARFBUZZ_VERSION   = $versions.HarfbuzzVersion
        PLAYER_HARFBUZZ_REF       = $versions.HarfbuzzRef
        PLAYER_HARFBUZZ_COMMIT    = $versions.HarfbuzzCommit
    }

    foreach ($entry in $environmentValues.GetEnumerator()) {
        $previousEnvironment[$entry.Key] = [Environment]::GetEnvironmentVariable($entry.Key, "Process")
        [Environment]::SetEnvironmentVariable($entry.Key, [string]$entry.Value, "Process")
    }

    $bash = Resolve-PlayerMsys2Bash
    Write-Host "Building audited libmpv package with MSYS2 CLANG64..."
    Write-Host "  Sources:  $($layout.DownloadsRootRelative)/sources"
    Write-Host "  Build:    $($layout.CacheRootRelative)"
    Write-Host "  Package:  $($layout.PackageRootRelative)"

    Invoke-PlayerClang64 -BashPath $bash -Command "bash scripts/libmpv/clang64/build-all.sh"

    Initialize-PlayerMsvcEnvironment -Versions $versions
    $importLibrary = New-PlayerLibMpvMsvcImportLibrary -Layout $layout
    $manifestPath = Write-PlayerLibMpvDependencyManifest -Versions $versions -Layout $layout
    $manifest = Test-PlayerLibMpvDependencyManifest -Versions $versions -Layout $layout

    Write-Host "[OK] MSVC import library: $importLibrary"
    Write-Host "[OK] dependency manifest: $manifestPath"
    Write-Host "[OK] runtime artifacts: $(@($manifest.artifacts).Count) hashed file(s)"
    Write-Host "libmpv $($versions.MpvVersion) package is ready at $($layout.PackageRootRelative)."
}
finally {
    foreach ($entry in $previousEnvironment.GetEnumerator()) {
        [Environment]::SetEnvironmentVariable($entry.Key, $entry.Value, "Process")
    }
    Pop-Location
}
