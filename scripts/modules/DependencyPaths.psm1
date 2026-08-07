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
    "Resolve-PlayerCMake",
    "Resolve-PlayerNinja"
)
