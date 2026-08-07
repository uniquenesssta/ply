Set-StrictMode -Version Latest

function Assert-PlayerRepositoryRelativePath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if ([System.IO.Path]::IsPathRooted($Path) -or -not $Path.StartsWith("../", [System.StringComparison]::Ordinal)) {
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
        QtRootRelative             = "../Qt/$($Versions.QtVersion)/msvc2022_64"
        LibMpvRootRelative         = "../libmpv/$($Versions.MpvVersion)/windows-x64"
        PortableCMakeRelative      = "../cmake/$($Versions.CMakeVersion)/bin/cmake.exe"
        PortableNinjaRelative      = "../ninja/$($Versions.NinjaVersion)/ninja.exe"
        DownloadsRootRelative      = "../downloads"
        FetchContentRootRelative   = "../cache/cmake/fetchcontent"
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
        [string]$RelativeCandidate,

        [Parameter(Mandatory = $true)]
        [string[]]$CommandNames
    )

    if (Test-Path -LiteralPath $RelativeCandidate -PathType Leaf) {
        return $RelativeCandidate
    }

    foreach ($commandName in $CommandNames) {
        $command = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($command) {
            return $commandName
        }
    }

    throw "$DisplayName was not found. Put the pinned tool on PATH or provide the optional relative copy at '$RelativeCandidate'."
}

function Resolve-PlayerCMake {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    return Resolve-PlayerToolCommand `
        -DisplayName "CMake" `
        -RelativeCandidate $Layout.PortableCMakeRelative `
        -CommandNames @("cmake.exe", "cmake")
}

function Resolve-PlayerNinja {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    return Resolve-PlayerToolCommand `
        -DisplayName "Ninja" `
        -RelativeCandidate $Layout.PortableNinjaRelative `
        -CommandNames @("ninja.exe", "ninja")
}

Export-ModuleMember -Function @(
    "Assert-PlayerRepositoryRelativePath",
    "Get-PlayerWorkspaceLayout",
    "Initialize-PlayerDependencyLayout",
    "Resolve-PlayerQtRoot",
    "Resolve-PlayerCMake",
    "Resolve-PlayerNinja"
)
