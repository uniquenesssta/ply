$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
$msvcModulePath = Join-Path $PSScriptRoot "modules/MsvcEnvironment.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force
Import-Module $msvcModulePath -Force

function Add-PlayerFailure {
    param(
        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [System.Collections.Generic.List[string]]$Failures,

        [Parameter(Mandatory = $true)]
        [string]$Message
    )

    [void]$Failures.Add($Message)
}

function Test-PlayerExactToolVersion {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$ExpectedVersion,

        [Parameter(Mandatory = $true)]
        [scriptblock]$ReadVersion,

        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [System.Collections.Generic.List[string]]$Failures
    )

    try {
        $actualVersion = [string]((& $ReadVersion) | Select-Object -First 1)
        $actualVersion = $actualVersion.Trim()
        if ($actualVersion -ne $ExpectedVersion) {
            Add-PlayerFailure -Failures $Failures -Message "$Name version mismatch. Expected $ExpectedVersion, found $actualVersion."
            Write-Host "[INVALID] ${Name}: $actualVersion (expected $ExpectedVersion)"
            return
        }

        Write-Host "[OK] ${Name}: $actualVersion"
    }
    catch {
        Add-PlayerFailure -Failures $Failures -Message $_.Exception.Message
        Write-Host "[MISSING] $Name"
    }
}

Push-Location $projectRoot
try {
    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $failures = [System.Collections.Generic.List[string]]::new()

    Write-Host "Workspace parent: .."
    Write-Host "  Qt:           $($layout.QtRootRelative)"
    Write-Host "  Qt tools:     $($layout.QtToolsRootRelative)"
    Write-Host "  libmpv:       $($layout.LibMpvRootRelative)"
    Write-Host "  Downloads:    $($layout.DownloadsRootRelative)"
    Write-Host "  FetchContent: $($layout.FetchContentRootRelative)"
    Write-Host "Version source: cmake/DependencyVersions.cmake"

    $cmake = $null
    try {
        $cmake = Resolve-PlayerCMake -Layout $layout
        Test-PlayerExactToolVersion `
            -Name "CMake" `
            -ExpectedVersion $versions.CMakeVersion `
            -Failures $failures `
            -ReadVersion {
                $line = [string](& $cmake --version | Select-Object -First 1)
                if ($line -notmatch '^cmake version ([0-9]+(?:\.[0-9]+)+)$') {
                    throw "Unable to parse CMake version output: $line"
                }
                $Matches[1]
            }
    }
    catch {
        Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
        Write-Host "[MISSING] CMake $($versions.CMakeVersion)"
    }

    $ninja = $null
    try {
        $ninja = Resolve-PlayerNinja -Layout $layout
        Test-PlayerExactToolVersion `
            -Name "Ninja" `
            -ExpectedVersion $versions.NinjaVersion `
            -Failures $failures `
            -ReadVersion { [string](& $ninja --version | Select-Object -First 1) }
    }
    catch {
        Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
        Write-Host "[MISSING] Ninja $($versions.NinjaVersion)"
    }

    try {
        $qtRoot = Resolve-PlayerQtRoot -Layout $layout
        $qmakeCandidates = @(
            (Join-Path $qtRoot "bin/qmake6.exe"),
            (Join-Path $qtRoot "bin/qmake.exe")
        )
        $qmake = $qmakeCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
        if (-not $qmake) {
            throw "qmake was not found under relative Qt kit '$qtRoot'."
        }

        Test-PlayerExactToolVersion `
            -Name "Qt" `
            -ExpectedVersion $versions.QtVersion `
            -Failures $failures `
            -ReadVersion { [string](& $qmake -query QT_VERSION | Select-Object -First 1) }
    }
    catch {
        Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
        Write-Host "[MISSING] Qt $($versions.QtVersion) MSVC 2022 x64 kit"
    }

    try {
        Initialize-PlayerMsvcEnvironment -Versions $versions
        Write-Host "[OK] Visual Studio x64 environment initialized"
    }
    catch {
        Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
        Write-Host "[MISSING] Visual Studio x64 build environment"
    }

    $compiler = Get-Command "cl.exe" -ErrorAction SilentlyContinue
    if (-not $compiler) {
        Add-PlayerFailure -Failures $failures -Message "cl.exe is unavailable after Visual Studio environment initialization."
        Write-Host "[MISSING] MSVC compiler"
    }
    else {
        $compilerBanner = ((& $compiler.Source 2>&1) | Select-Object -First 1) -as [string]
        if ($compilerBanner -notmatch 'Version\s+([0-9]+\.[0-9]+)(?:\.[0-9]+)?') {
            Add-PlayerFailure -Failures $failures -Message "Unable to parse cl.exe version output: $compilerBanner"
            Write-Host "[INVALID] MSVC compiler version output"
        }
        elseif ($Matches[1] -ne $versions.MsvcCompilerVersion) {
            Add-PlayerFailure -Failures $failures -Message "MSVC compiler family mismatch. Expected $($versions.MsvcCompilerVersion), found $($Matches[1])."
            Write-Host "[INVALID] MSVC compiler: $($Matches[1]) (expected $($versions.MsvcCompilerVersion))"
        }
        else {
            Write-Host "[OK] MSVC compiler family: $($Matches[1]) / toolset $($versions.MsvcToolsetVersion)"
        }
    }

    if ([string]::IsNullOrWhiteSpace($env:VSCMD_VER)) {
        Add-PlayerFailure -Failures $failures -Message "VSCMD_VER is not initialized after automatic Visual Studio environment setup."
        Write-Host "[MISSING] Visual Studio developer environment"
    }
    else {
        try {
            $vswhere = Get-PlayerVsWherePath
            $parsedVisualStudioVersion = [version]$versions.VisualStudioVersion
            $lower = "$($parsedVisualStudioVersion.Major).$($parsedVisualStudioVersion.Minor)"
            $upper = "$($parsedVisualStudioVersion.Major).$($parsedVisualStudioVersion.Minor + 1)"
            $installationVersion = [string](& $vswhere -latest -products * -version "[$lower,$upper)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion | Select-Object -First 1)
            $installationVersion = $installationVersion.Trim()
            if ([string]::IsNullOrWhiteSpace($installationVersion)) {
                Add-PlayerFailure -Failures $failures -Message "Visual Studio 2022 release $($versions.VisualStudioVersion) with the C++ x64 tools was not found."
                Write-Host "[MISSING] Visual Studio $($versions.VisualStudioVersion)"
            }
            elseif ($installationVersion -ne $versions.VisualStudioBuild) {
                Add-PlayerFailure -Failures $failures -Message "Visual Studio build mismatch. Expected $($versions.VisualStudioBuild) (release $($versions.VisualStudioVersion)), found $installationVersion."
                Write-Host "[INVALID] Visual Studio: $installationVersion (expected $($versions.VisualStudioBuild))"
            }
            else {
                Write-Host "[OK] Visual Studio: $($versions.VisualStudioVersion) / $installationVersion"
            }
        }
        catch {
            Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
            Write-Host "[MISSING] Visual Studio version verifier"
        }
    }

    $targetArchitecture = $env:VSCMD_ARG_TGT_ARCH
    if ($targetArchitecture -ne "x64") {
        $reportedArchitecture = if ([string]::IsNullOrWhiteSpace($targetArchitecture)) { "not initialized" } else { $targetArchitecture }
        Add-PlayerFailure -Failures $failures -Message "The active MSVC target architecture is '$reportedArchitecture'; x64 is required."
        Write-Host "[INVALID] MSVC target architecture: $reportedArchitecture"
    }
    else {
        Write-Host "[OK] MSVC target architecture: x64"
    }

    $windowsSdkVersion = if ($env:WindowsSDKVersion) { $env:WindowsSDKVersion.TrimEnd([char]92) } else { "" }
    if ($windowsSdkVersion -ne $versions.WindowsSdkVersion) {
        $reportedSdk = if ($windowsSdkVersion) { $windowsSdkVersion } else { "not initialized" }
        Add-PlayerFailure -Failures $failures -Message "Windows SDK mismatch. Expected $($versions.WindowsSdkVersion) from release $($versions.WindowsSdkRelease), found $reportedSdk."
        Write-Host "[INVALID] Windows SDK: $reportedSdk (expected $($versions.WindowsSdkVersion))"
    }
    else {
        Write-Host "[OK] Windows SDK: $windowsSdkVersion (release $($versions.WindowsSdkRelease))"
    }

    $osVersion = [System.Environment]::OSVersion.Version
    $minimumBuild = [version]$versions.WindowsMinBuild
    if ($osVersion -lt $minimumBuild) {
        Add-PlayerFailure -Failures $failures -Message "Windows build $osVersion is below the supported minimum $($versions.WindowsMinBuild)."
        Write-Host "[INVALID] Windows: $osVersion"
    }
    else {
        Write-Host "[OK] Windows: $osVersion (minimum $($versions.WindowsMinBuild), primary validation $($versions.WindowsPrimaryBuild))"
    }

    $libMpvHeader = Join-Path $layout.LibMpvRootRelative "include/mpv/client.h"
    if (Test-Path -LiteralPath $libMpvHeader -PathType Leaf) {
        $manifestPath = Join-Path $layout.LibMpvRootRelative "dependency-manifest.json"
        if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
            Add-PlayerFailure -Failures $failures -Message "libmpv files exist but dependency-manifest.json is missing at '$manifestPath'."
            Write-Host "[INVALID] libmpv dependency manifest missing"
        }
        else {
            try {
                $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
                $manifestChecks = @(
                    @("mpv.version", [string]$manifest.mpv.version, $versions.MpvVersion),
                    @("mpv.tag", [string]$manifest.mpv.tag, $versions.MpvTag),
                    @("mpv.commit", [string]$manifest.mpv.commit, $versions.MpvCommit),
                    @("ffmpeg.version", [string]$manifest.ffmpeg.version, $versions.FfmpegVersion)
                )
                foreach ($check in $manifestChecks) {
                    if ($check[1] -ne $check[2]) {
                        Add-PlayerFailure -Failures $failures -Message "libmpv manifest $($check[0]) mismatch. Expected $($check[2]), found $($check[1])."
                    }
                }
                Write-Host "[OK] libmpv manifest: mpv $($manifest.mpv.version), FFmpeg $($manifest.ffmpeg.version)"
            }
            catch {
                Add-PlayerFailure -Failures $failures -Message "Unable to read libmpv dependency manifest: $($_.Exception.Message)"
                Write-Host "[INVALID] libmpv dependency manifest"
            }
        }
    }
    else {
        Write-Host "[INFO] libmpv $($versions.MpvVersion) SDK is not installed yet; it becomes required in Stage R2."
    }

    if ($failures.Count -gt 0) {
        $failureList = ($failures | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
        throw @"
Dependency verification failed:
$failureList
"@
    }

    Write-Host "Pinned dependencies required by the current scaffold are available and version-matched."
}
finally {
    Pop-Location
}
