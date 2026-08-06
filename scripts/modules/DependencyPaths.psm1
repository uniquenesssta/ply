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
        [string]$ProjectRoot,

        [Parameter(Mandatory = $true)]
        [psobject]$Versions
    )

    $resolvedProjectRoot = (Resolve-Path -LiteralPath $ProjectRoot).Path
    $parentRoot = [System.IO.Path]::GetFullPath((Join-Path $resolvedProjectRoot ".."))

    $relative = [ordered]@{
        QtRoot           = "../Qt/$($Versions.QtVersion)/msvc2022_64"
        LibMpvRoot       = "../libmpv/$($Versions.MpvVersion)/windows-x64"
        CMakeRoot        = "../cmake/$($Versions.CMakeVersion)"
        NinjaRoot        = "../ninja/$($Versions.NinjaVersion)"
        DownloadsRoot    = "../downloads"
        FetchContentRoot = "../cache/cmake/fetchcontent"
    }

    foreach ($entry in $relative.GetEnumerator()) {
        Assert-PlayerRepositoryRelativePath -Name $entry.Key -Path $entry.Value
    }

    [pscustomobject]@{
        ProjectRoot              = $resolvedProjectRoot
        ParentRoot               = $parentRoot
        QtRootRelative           = $relative.QtRoot
        LibMpvRootRelative       = $relative.LibMpvRoot
        CMakeRootRelative        = $relative.CMakeRoot
        NinjaRootRelative        = $relative.NinjaRoot
        DownloadsRootRelative    = $relative.DownloadsRoot
        FetchContentRootRelative = $relative.FetchContentRoot
        QtRoot                   = Join-Path $parentRoot "Qt/$($Versions.QtVersion)/msvc2022_64"
        LibMpvRoot               = Join-Path $parentRoot "libmpv/$($Versions.MpvVersion)/windows-x64"
        CMakeRoot                = Join-Path $parentRoot "cmake/$($Versions.CMakeVersion)"
        NinjaRoot                = Join-Path $parentRoot "ninja/$($Versions.NinjaVersion)"
        DownloadsRoot            = Join-Path $parentRoot "downloads"
        FetchContentRoot         = Join-Path $parentRoot "cache/cmake/fetchcontent"
    }
}

function Initialize-PlayerDependencyLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    # Only project-owned shared staging/cache directories are created. Installed
    # Qt, CMake, and Ninja directories are never created or modified here.
    foreach ($directory in @(
        $Layout.LibMpvRoot,
        $Layout.DownloadsRoot,
        $Layout.FetchContentRoot
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

    $configFile = Join-Path $Layout.QtRoot "lib/cmake/Qt6/Qt6Config.cmake"
    if (-not (Test-Path -LiteralPath $configFile -PathType Leaf)) {
        throw "Pinned Qt kit was not found. Expected: $configFile"
    }
    return $Layout.QtRoot
}

function Resolve-PlayerCMake {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $executable = Join-Path $Layout.CMakeRoot "bin/cmake.exe"
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
        throw "Pinned CMake executable was not found. Expected: $executable"
    }
    return (Resolve-Path -LiteralPath $executable).Path
}

function Resolve-PlayerNinja {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $executable = Join-Path $Layout.NinjaRoot "ninja.exe"
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
        throw "Pinned Ninja executable was not found. Expected: $executable"
    }
    return (Resolve-Path -LiteralPath $executable).Path
}

Export-ModuleMember -Function @(
    "Get-PlayerWorkspaceLayout",
    "Initialize-PlayerDependencyLayout",
    "Resolve-PlayerQtRoot",
    "Resolve-PlayerCMake",
    "Resolve-PlayerNinja"
)
