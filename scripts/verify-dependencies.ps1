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

function Test-PlayerMinimumToolVersion {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$MinimumVersion,

        [Parameter(Mandatory = $true)]
        [scriptblock]$ReadVersion,

        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [System.Collections.Generic.List[string]]$Failures
    )

    try {
        $actualVersionText = [string]((& $ReadVersion) | Select-Object -First 1)
        $actualVersionText = $actualVersionText.Trim()
        $actualVersion = [version]$actualVersionText
        $minimum = [version]$MinimumVersion
        if ($actualVersion -lt $minimum) {
            Add-PlayerFailure -Failures $Failures -Message "$Name is too old. Minimum $MinimumVersion, found $actualVersionText."
            Write-Host "[INVALID] ${Name}: $actualVersionText (minimum $MinimumVersion)"
            return
        }

        Write-Host "[OK] ${Name}: $actualVersionText (minimum $MinimumVersion)"
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
        Test-PlayerMinimumToolVersion `
            -Name "CMake" `
            -MinimumVersion $versions.CMakeVersion `
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
        Write-Host "[MISSING] CMake $($versions.CMakeVersion)+"
    }

    $ninja = $null
    try {
        $ninja = Resolve-PlayerNinja -Layout $layout
        Test-PlayerMinimumToolVersion `
            -Name "Ninja" `
            -MinimumVersion $versions.NinjaVersion `
            -Failures $failures `
            -ReadVersion { [string](& $ninja --version | Select-Object -First 1) }
    }
    catch {
        Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
        Write-Host "[MISSING] Ninja $($versions.NinjaVersion)+"
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
        try {
            $compilerFileVersion = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($compiler.Source).FileVersion
            if ($compilerFileVersion -notmatch '^([0-9]+\.[0-9]+)') {
                throw "Unable to parse cl.exe file version: $compilerFileVersion"
            }

            $compilerFamily = $Matches[1]
            if ($compilerFamily -ne $versions.MsvcCompilerVersion) {
                Add-PlayerFailure -Failures $failures -Message "MSVC compiler family mismatch. Expected $($versions.MsvcCompilerVersion), found $compilerFamily ($compilerFileVersion)."
                Write-Host "[INVALID] MSVC compiler: $compilerFileVersion (expected family $($versions.MsvcCompilerVersion))"
            }
            else {
                Write-Host "[OK] MSVC compiler: $compilerFileVersion (family $compilerFamily)"
            }
        }
        catch {
            Add-PlayerFailure -Failures $failures -Message $_.Exception.Message
            Write-Host "[INVALID] MSVC compiler version"
        }
    }

    $vcToolsVersion = if ($env:VCToolsVersion) { $env:VCToolsVersion.TrimEnd([char]92) } else { "" }
    if ([string]::IsNullOrWhiteSpace($vcToolsVersion)) {
        Add-PlayerFailure -Failures $failures -Message "VCToolsVersion is not initialized after automatic Visual Studio environment setup."
        Write-Host "[MISSING] MSVC toolset version"
    }
    elseif (-not ($vcToolsVersion -eq $versions.MsvcToolsetVersion -or $vcToolsVersion.StartsWith("$($versions.MsvcToolsetVersion).", [System.StringComparison]::Ordinal))) {
        Add-PlayerFailure -Failures $failures -Message "MSVC toolset family mismatch. Expected $($versions.MsvcToolsetVersion), found $vcToolsVersion."
        Write-Host "[INVALID] MSVC toolset: $vcToolsVersion (expected family $($versions.MsvcToolsetVersion))"
    }
    else {
        Write-Host "[OK] MSVC toolset: $vcToolsVersion (family $($versions.MsvcToolsetVersion))"
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
            $installationVersionText = [string](& $vswhere -latest -products * -version "[$lower,$upper)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion | Select-Object -First 1)
            $installationVersionText = $installationVersionText.Trim()
            if ([string]::IsNullOrWhiteSpace($installationVersionText)) {
                Add-PlayerFailure -Failures $failures -Message "Visual Studio 2022 $lower family with the C++ x64 tools was not found."
                Write-Host "[MISSING] Visual Studio $lower family"
            }
            else {
                $installationVersion = [version]$installationVersionText
                if ($installationVersion.Major -ne $parsedVisualStudioVersion.Major -or $installationVersion.Minor -ne $parsedVisualStudioVersion.Minor) {
                    Add-PlayerFailure -Failures $failures -Message "Visual Studio family mismatch. Expected $lower, found $installationVersionText."
                    Write-Host "[INVALID] Visual Studio: $installationVersionText (expected $lower family)"
                }
                else {
                    Write-Host "[OK] Visual Studio: $installationVersionText ($lower family; reference $($versions.VisualStudioBuild))"
                }
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
    if ([string]::IsNullOrWhiteSpace($windowsSdkVersion)) {
        Add-PlayerFailure -Failures $failures -Message "WindowsSDKVersion is not initialized after automatic Visual Studio environment setup."
        Write-Host "[MISSING] Windows SDK"
    }
    else {
        try {
            $actualSdkVersion = [version]$windowsSdkVersion
            $minimumSdkVersion = [version]$versions.WindowsSdkVersion
            if ($actualSdkVersion -lt $minimumSdkVersion) {
                Add-PlayerFailure -Failures $failures -Message "Windows SDK is too old. Minimum $($versions.WindowsSdkVersion), found $windowsSdkVersion."
                Write-Host "[INVALID] Windows SDK: $windowsSdkVersion (minimum $($versions.WindowsSdkVersion))"
            }
            else {
                Write-Host "[OK] Windows SDK: $windowsSdkVersion (minimum $($versions.WindowsSdkVersion); reference release $($versions.WindowsSdkRelease))"
            }
        }
        catch {
            Add-PlayerFailure -Failures $failures -Message "Unable to parse Windows SDK version '$windowsSdkVersion'."
            Write-Host "[INVALID] Windows SDK version output"
        }
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

    try {
        $libMpvPackage = Resolve-PlayerLibMpvPackage -Layout $layout
        $manifest = Get-Content -LiteralPath $libMpvPackage.ManifestRelative -Raw | ConvertFrom-Json
        $manifestChecks = @(
            @("mpv.version", [string]$manifest.mpv.version, $versions.MpvVersion),
            @("mpv.tag", [string]$manifest.mpv.tag, $versions.MpvTag),
            @("mpv.commit", [string]$manifest.mpv.commit, $versions.MpvCommit),
            @("ffmpeg.version", [string]$manifest.ffmpeg.version, $versions.FfmpegVersion)
        )

        $manifestValid = $true
        foreach ($check in $manifestChecks) {
            if ($check[1] -ne $check[2]) {
                $manifestValid = $false
                Add-PlayerFailure -Failures $failures -Message "libmpv manifest $($check[0]) mismatch. Expected $($check[2]), found $($check[1])."
            }
        }

        if ($manifestValid) {
            Write-Host "[OK] libmpv manifest: mpv $($manifest.mpv.version), tag $($manifest.mpv.tag), FFmpeg $($manifest.ffmpeg.version)"
        }
        else {
            Write-Host "[INVALID] libmpv dependency manifest identity"
        }

        Write-Host "[OK] libmpv header: $($libMpvPackage.HeaderRelative)"
        Write-Host "[OK] libmpv import library: $($libMpvPackage.ImportLibraryRelative)"
        Write-Host "[OK] libmpv runtime DLL: $($libMpvPackage.RuntimeLibraryRelative)"

        $runtimeHash = (Get-FileHash -LiteralPath $libMpvPackage.RuntimeLibraryRelative -Algorithm SHA256).Hash.ToLowerInvariant()
        $importHash = (Get-FileHash -LiteralPath $libMpvPackage.ImportLibraryRelative -Algorithm SHA256).Hash.ToLowerInvariant()
        Write-Host "[INFO] libmpv runtime SHA-256: $runtimeHash"
        Write-Host "[INFO] libmpv import SHA-256:  $importHash"
    }
    catch {
        Add-PlayerFailure -Failures $failures -Message "libmpv package verification failed: $($_.Exception.Message)"
        Write-Host "[MISSING] required libmpv $($versions.MpvVersion) SDK/runtime package"
    }

    if ($failures.Count -gt 0) {
        $failureList = ($failures | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
        throw @"
Dependency verification failed:
$failureList
"@
    }

    Write-Host "Compatible development tools and required R2 product dependencies are available."
}
finally {
    Pop-Location
}
