Set-StrictMode -Version Latest

function Assert-PlayerDevelopmentRuntimeMarker {
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

    if (-not (Test-Path -LiteralPath $markerPath -PathType Leaf)) {
        throw "Development runtime marker is missing at '$markerPath'. Re-run scripts/configure.ps1 before building."
    }

    $markerValue = (Get-Content -LiteralPath $markerPath -Raw).Trim()
    if ($markerValue -ne "../..") {
        throw "Development runtime marker contains an unexpected relative root: '$markerValue'."
    }

    $resolvedRoot = (Resolve-Path -LiteralPath (Join-Path $outputDirectory $markerValue)).Path
    if (-not [string]::Equals(
        $resolvedRoot,
        $projectRootPath,
        [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Development runtime marker resolves outside the active project root: '$resolvedRoot'."
    }

    Write-Host "[OK] Development runtime root marker -> build/$Preset/.player-development-root"
}

Export-ModuleMember -Function "Assert-PlayerDevelopmentRuntimeMarker"
