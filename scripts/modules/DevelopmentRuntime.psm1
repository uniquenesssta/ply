Set-StrictMode -Version Latest

function Get-PlayerDevelopmentRuntimePaths {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
        [string]$Preset,

        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $projectRootPath = (Resolve-Path -LiteralPath $ProjectRoot).Path
    $outputDirectory = Join-Path $projectRootPath "build/$Preset"
    $markerPath = Join-Path $outputDirectory ".player-development-root"
    $executablePath = Join-Path $outputDirectory "Player.exe"

    return [pscustomobject]@{
        ProjectRoot = $projectRootPath
        OutputDirectory = $outputDirectory
        MarkerPath = $markerPath
        ExecutablePath = $executablePath
    }
}

function Assert-PlayerDevelopmentRuntimeMarker {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
        [string]$Preset,

        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $paths = Get-PlayerDevelopmentRuntimePaths -Preset $Preset -ProjectRoot $ProjectRoot

    if (-not (Test-Path -LiteralPath $paths.ExecutablePath -PathType Leaf)) {
        throw "Player executable is missing at '$($paths.ExecutablePath)'."
    }
    if (-not (Test-Path -LiteralPath $paths.MarkerPath -PathType Leaf)) {
        throw "Development runtime marker is missing at '$($paths.MarkerPath)'."
    }

    $markerValue = (Get-Content -LiteralPath $paths.MarkerPath -Raw).Trim()
    if ($markerValue -ne "../.." -or [System.IO.Path]::IsPathRooted($markerValue)) {
        throw "Development runtime marker contains an unexpected root route: '$markerValue'."
    }

    $resolvedRoot = [System.IO.Path]::GetFullPath((Join-Path $paths.OutputDirectory $markerValue))
    if (-not [string]::Equals(
        $resolvedRoot.TrimEnd('\', '/'),
        $paths.ProjectRoot.TrimEnd('\', '/'),
        [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Development runtime marker resolves outside the active project root: '$resolvedRoot'."
    }

    Write-Host "[OK] Development runtime root marker -> build/$Preset/.player-development-root"
}

function Set-PlayerDevelopmentRuntimeMarker {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
        [string]$Preset,

        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $paths = Get-PlayerDevelopmentRuntimePaths -Preset $Preset -ProjectRoot $ProjectRoot
    if (-not (Test-Path -LiteralPath $paths.ExecutablePath -PathType Leaf)) {
        throw "Player executable is missing at '$($paths.ExecutablePath)'; refusing to create a marker for a stale or incomplete build."
    }

    $utf8NoBom = [System.Text.UTF8Encoding]::new($false)
    [System.IO.File]::WriteAllText(
        $paths.MarkerPath,
        "../.." + [Environment]::NewLine,
        $utf8NoBom)

    Assert-PlayerDevelopmentRuntimeMarker -Preset $Preset -ProjectRoot $ProjectRoot
}

Export-ModuleMember -Function @(
    "Assert-PlayerDevelopmentRuntimeMarker",
    "Set-PlayerDevelopmentRuntimeMarker"
)
