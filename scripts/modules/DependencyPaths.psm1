Set-StrictMode -Version Latest

function Assert-PlayerRepositoryRelativePath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $normalizedPath = $Path -replace '\\', '/'
    if ([System.IO.Path]::IsPathRooted($Path) -or -not $normalizedPath.StartsWith("../", [System.StringComparison]::Ordinal)) {
        throw "$Name must begin with ../ and must not be absolute; received '$Path'."
    }
}

function Get-PlayerWorkspaceLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Versions
    )

    $layout = [pscustomobject]@{
        QtRootRelative           = "../Qt/$($Versions.QtVersion)/msvc2022_64"
        QtToolsRootRelative      = "../Qt/Tools"
        LibMpvRootRelative       = "../libmpv/$($Versions.MpvVersion)/windows-x64"
        DownloadsRootRelative    = "../downloads"
        FetchContentRootRelative = "../cache/cmake/fetchcontent"
    }

    foreach ($entry in $layout.PSObject.Properties) {
        Assert-PlayerRepositoryRelativePath -Name $entry.Name -Path ([string]$entry.Value)
    }

    return $layout
}

function Initialize-PlayerDependencyLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    foreach ($directory in @(
        $Layout.LibMpvRootRelative,
        $Layout.DownloadsRootRelative,
        $Layout.FetchContentRootRelative
    )) {
        [System.IO.Directory]::CreateDirectory($directory) | Out-Null
    }
}

function Resolve-PlayerQtRoot {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $configFile = Join-Path $Layout.QtRootRelative "lib/cmake/Qt6/Qt6Config.cmake"
    if (-not (Test-Path -LiteralPath $configFile -PathType Leaf)) {
        throw "Pinned Qt kit was not found at relative path '$($Layout.QtRootRelative)'."
    }

    return $Layout.QtRootRelative
}

function Resolve-PlayerSingleExistingFile {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$DisplayName,

        [Parameter(Mandatory = $true)]
        [string[]]$RelativeCandidates
    )

    foreach ($candidate in $RelativeCandidates) {
        Assert-PlayerRepositoryRelativePath -Name "$DisplayName candidate" -Path $candidate
    }

    $matches = @($RelativeCandidates | Where-Object {
        Test-Path -LiteralPath $_ -PathType Leaf
    })

    if ($matches.Count -eq 0) {
        $candidateList = $RelativeCandidates -join "', '"
        throw "$DisplayName was not found in the fixed parent-workspace package. Checked relative path(s): '$candidateList'."
    }

    if ($matches.Count -gt 1) {
        $matchList = $matches -join "', '"
        throw "$DisplayName is ambiguous inside the fixed libmpv package. Keep one audited artifact only; found '$matchList'."
    }

    return [string]$matches[0]
}

function Resolve-PlayerLibMpvPackage {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $root = [string]$Layout.LibMpvRootRelative
    Assert-PlayerRepositoryRelativePath -Name "libmpv root" -Path $root

    $header = Join-Path $root "include/mpv/client.h"
    $manifest = Join-Path $root "dependency-manifest.json"
    foreach ($requiredFile in @(
        @("libmpv client header", $header),
        @("libmpv dependency manifest", $manifest)
    )) {
        Assert-PlayerRepositoryRelativePath -Name $requiredFile[0] -Path $requiredFile[1]
        if (-not (Test-Path -LiteralPath $requiredFile[1] -PathType Leaf)) {
            throw "$($requiredFile[0]) was not found at relative path '$($requiredFile[1])'."
        }
    }

    $importLibrary = Resolve-PlayerSingleExistingFile `
        -DisplayName "libmpv MSVC import library" `
        -RelativeCandidates @(
            (Join-Path $root "lib/mpv.lib"),
            (Join-Path $root "lib/libmpv.lib"),
            (Join-Path $root "lib/mpv-2.lib"),
            (Join-Path $root "lib/libmpv-2.lib"),
            (Join-Path $root "mpv.lib"),
            (Join-Path $root "libmpv.lib"),
            (Join-Path $root "mpv-2.lib"),
            (Join-Path $root "libmpv-2.lib")
        )

    $runtimeLibrary = Resolve-PlayerSingleExistingFile `
        -DisplayName "libmpv runtime DLL" `
        -RelativeCandidates @(
            (Join-Path $root "bin/libmpv-2.dll"),
            (Join-Path $root "bin/mpv-2.dll"),
            (Join-Path $root "libmpv-2.dll"),
            (Join-Path $root "mpv-2.dll")
        )

    [pscustomobject]@{
        RootRelative          = $root
        HeaderRelative        = $header
        ManifestRelative      = $manifest
        ImportLibraryRelative = $importLibrary
        RuntimeLibraryRelative = $runtimeLibrary
    }
}

function Resolve-PlayerToolCommand {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$DisplayName,

        [Parameter(Mandatory = $true)]
        [string[]]$CommandNames,

        [Parameter(Mandatory = $true)]
        [string[]]$RelativeCandidates
    )

    foreach ($commandName in $CommandNames) {
        $command = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($command) {
            return $commandName
        }
    }

    foreach ($relativeCandidate in $RelativeCandidates) {
        Assert-PlayerRepositoryRelativePath -Name "$DisplayName candidate" -Path $relativeCandidate
        if (Test-Path -LiteralPath $relativeCandidate -PathType Leaf) {
            return $relativeCandidate
        }
    }

    $candidateList = $RelativeCandidates -join "', '"
    throw "$DisplayName was not found on PATH or in the existing Qt tools tree. Checked relative path(s): '$candidateList'."
}

function Resolve-PlayerCMake {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    return Resolve-PlayerToolCommand `
        -DisplayName "CMake" `
        -CommandNames @("cmake.exe", "cmake") `
        -RelativeCandidates @(
            (Join-Path $Layout.QtToolsRootRelative "CMake_64/bin/cmake.exe")
        )
}

function Resolve-PlayerCTest {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    return Resolve-PlayerToolCommand `
        -DisplayName "CTest" `
        -CommandNames @("ctest.exe", "ctest") `
        -RelativeCandidates @(
            (Join-Path $Layout.QtToolsRootRelative "CMake_64/bin/ctest.exe")
        )
}

function Resolve-PlayerNinja {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    return Resolve-PlayerToolCommand `
        -DisplayName "Ninja" `
        -CommandNames @("ninja.exe", "ninja") `
        -RelativeCandidates @(
            (Join-Path $Layout.QtToolsRootRelative "Ninja/ninja.exe")
        )
}

Export-ModuleMember -Function @(
    "Assert-PlayerRepositoryRelativePath",
    "Get-PlayerWorkspaceLayout",
    "Initialize-PlayerDependencyLayout",
    "Resolve-PlayerQtRoot",
    "Resolve-PlayerSingleExistingFile",
    "Resolve-PlayerLibMpvPackage",
    "Resolve-PlayerCMake",
    "Resolve-PlayerCTest",
    "Resolve-PlayerNinja"
)
