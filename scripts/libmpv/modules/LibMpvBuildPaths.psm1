Set-StrictMode -Version Latest

function Assert-PlayerLibMpvRelativePath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $normalized = $Path -replace '\\', '/'
    if ([System.IO.Path]::IsPathRooted($Path) -or -not $normalized.StartsWith("../", [System.StringComparison]::Ordinal)) {
        throw "$Name must remain repository-parent-relative and begin with ../; received '$Path'."
    }
}

function Get-PlayerLibMpvBuildLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Versions
    )

    $layout = [pscustomobject]@{
        DownloadsRootRelative = "../downloads/libmpv"
        CacheRootRelative     = "../cache/libmpv-build"
        PackageRootRelative   = "../libmpv/$($Versions.MpvVersion)/windows-x64"
    }

    foreach ($entry in $layout.PSObject.Properties) {
        Assert-PlayerLibMpvRelativePath -Name $entry.Name -Path ([string]$entry.Value)
    }

    return $layout
}

function Initialize-PlayerLibMpvBuildLayout {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    foreach ($path in @(
        $Layout.DownloadsRootRelative,
        $Layout.CacheRootRelative,
        $Layout.PackageRootRelative
    )) {
        [System.IO.Directory]::CreateDirectory($path) | Out-Null
    }
}

Export-ModuleMember -Function @(
    "Get-PlayerLibMpvBuildLayout",
    "Initialize-PlayerLibMpvBuildLayout"
)
