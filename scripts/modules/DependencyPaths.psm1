Set-StrictMode -Version Latest

function Get-PlayerWorkspaceLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $resolvedProjectRoot = (Resolve-Path -LiteralPath $ProjectRoot).Path
    $parentRoot = [System.IO.Path]::GetFullPath((Join-Path $resolvedProjectRoot ".."))

    [pscustomobject]@{
        ProjectRoot   = $resolvedProjectRoot
        ParentRoot    = $parentRoot
        QtBaseRoot    = Join-Path $parentRoot "Qt"
        QtToolsRoot   = Join-Path $parentRoot "Qt/Tools"
        LibMpvRoot    = Join-Path $parentRoot "libmpv/windows-x64"
        CMakeRoot     = Join-Path $parentRoot "cmake"
        NinjaRoot     = Join-Path $parentRoot "ninja"
        DownloadsRoot = Join-Path $parentRoot "downloads"
        CacheRoot     = Join-Path $parentRoot "cache"
    }
}

function Initialize-PlayerDependencyLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    # Do not create or modify Qt, CMake, or Ninja installation directories.
    # Only create project-owned shared locations directly in the repository parent.
    $directories = @(
        $Layout.LibMpvRoot,
        $Layout.DownloadsRoot,
        (Join-Path $Layout.CacheRoot "cmake/fetchcontent")
    )

    foreach ($directory in $directories) {
        [System.IO.Directory]::CreateDirectory($directory) | Out-Null
    }
}

function Test-PlayerQtRoot {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    Test-Path -LiteralPath (Join-Path $Path "lib/cmake/Qt6/Qt6Config.cmake") -PathType Leaf
}

function ConvertTo-PlayerRepositoryRelativePath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout,

        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    Push-Location $Layout.ProjectRoot
    try {
        $relativePath = Resolve-Path -LiteralPath $Path -Relative
        return ($relativePath -replace '\\', '/')
    }
    finally {
        Pop-Location
    }
}

function Resolve-PlayerQtRoot {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $candidates = [System.Collections.Generic.List[string]]::new()

    foreach ($baseRoot in @($Layout.QtBaseRoot, (Join-Path $Layout.ParentRoot "qt"))) {
        if (-not (Test-Path -LiteralPath $baseRoot -PathType Container)) {
            continue
        }

        $directKit = Join-Path $baseRoot "msvc2022_64"
        if (Test-PlayerQtRoot -Path $directKit) {
            $candidates.Add($directKit)
        }

        Get-ChildItem -LiteralPath $baseRoot -Directory -ErrorAction SilentlyContinue |
            Sort-Object -Property @{
                Expression = {
                    try { [version]$_.Name } catch { [version]"0.0" }
                }
                Descending = $true
            }, @{
                Expression = { $_.Name }
                Descending = $true
            } |
            ForEach-Object {
                $kit = Join-Path (Join-Path $baseRoot $_.Name) "msvc2022_64"
                if (Test-PlayerQtRoot -Path $kit) {
                    $candidates.Add($kit)
                }
            }
    }

    $selected = $candidates | Select-Object -First 1
    if ($selected) {
        return ConvertTo-PlayerRepositoryRelativePath -Layout $Layout -Path $selected
    }

    throw @"
Qt 6 MSVC 2022 x64 kit was not found.
Expected relative layout:
  ../Qt/<version>/msvc2022_64
"@
}

function Resolve-PlayerExecutable {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$CommandName,

        [string[]]$PreferredPaths = @()
    )

    foreach ($candidate in $PreferredPaths) {
        if (-not [string]::IsNullOrWhiteSpace($candidate) -and
            (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    $command = Get-Command $CommandName -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    throw "Required executable '$CommandName' was not found in the repository parent directory or on PATH."
}

function Get-PlayerVersionedToolCandidates {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$ToolRoot,

        [Parameter(Mandatory = $true)]
        [string]$RelativeExecutablePath
    )

    if (-not (Test-Path -LiteralPath $ToolRoot -PathType Container)) {
        return @()
    }

    @(
        Get-ChildItem -LiteralPath $ToolRoot -Directory -ErrorAction SilentlyContinue |
            Sort-Object -Property @{
                Expression = {
                    try { [version]$_.Name } catch { [version]"0.0" }
                }
                Descending = $true
            }, @{
                Expression = { $_.Name }
                Descending = $true
            } |
            ForEach-Object {
                Join-Path (Join-Path $ToolRoot $_.Name) $RelativeExecutablePath
            }
    )
}

function Resolve-PlayerCMake {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $versionedCandidates = Get-PlayerVersionedToolCandidates `
        -ToolRoot $Layout.CMakeRoot `
        -RelativeExecutablePath "bin/cmake.exe"

    $qtToolCandidates = @()
    if (Test-Path -LiteralPath $Layout.QtToolsRoot -PathType Container) {
        $qtToolCandidates = @(
            Get-ChildItem -LiteralPath $Layout.QtToolsRoot -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -like "CMake*" } |
                Sort-Object -Property Name -Descending |
                ForEach-Object {
                    Join-Path $_.FullName "bin/cmake.exe"
                }
        )
    }

    $preferredPaths = @(
        (Join-Path $Layout.CMakeRoot "bin/cmake.exe"),
        (Join-Path $Layout.CMakeRoot "cmake.exe"),
        (Join-Path $Layout.QtToolsRoot "CMake_64/bin/cmake.exe")
    ) + $versionedCandidates + $qtToolCandidates

    Resolve-PlayerExecutable -CommandName "cmake" -PreferredPaths $preferredPaths
}

function Resolve-PlayerNinja {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $versionedCandidates = Get-PlayerVersionedToolCandidates `
        -ToolRoot $Layout.NinjaRoot `
        -RelativeExecutablePath "ninja.exe"

    $preferredPaths = @(
        (Join-Path $Layout.NinjaRoot "ninja.exe"),
        (Join-Path $Layout.QtToolsRoot "Ninja/ninja.exe")
    ) + $versionedCandidates

    Resolve-PlayerExecutable -CommandName "ninja" -PreferredPaths $preferredPaths
}

Export-ModuleMember -Function @(
    "Get-PlayerWorkspaceLayout",
    "Initialize-PlayerDependencyLayout",
    "Resolve-PlayerQtRoot",
    "Resolve-PlayerCMake",
    "Resolve-PlayerNinja"
)
