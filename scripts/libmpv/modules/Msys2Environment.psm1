Set-StrictMode -Version Latest

function Test-PlayerMsys2BashPath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return $false
    }

    $normalized = $Path -replace '/', '\\'
    return $normalized.EndsWith("\usr\bin\bash.exe", [System.StringComparison]::OrdinalIgnoreCase)
}

function Resolve-PlayerMsys2Bash {
    [CmdletBinding()]
    param()

    $candidates = [System.Collections.Generic.List[string]]::new()

    if (-not [string]::IsNullOrWhiteSpace($env:MSYS2_ROOT)) {
        [void]$candidates.Add((Join-Path $env:MSYS2_ROOT "usr/bin/bash.exe"))
    }

    $bashCommand = Get-Command "bash.exe" -ErrorAction SilentlyContinue
    if ($bashCommand -and -not [string]::IsNullOrWhiteSpace($bashCommand.Source)) {
        [void]$candidates.Add($bashCommand.Source)
    }

    if (-not [string]::IsNullOrWhiteSpace($env:SystemDrive)) {
        [void]$candidates.Add((Join-Path $env:SystemDrive "msys64/usr/bin/bash.exe"))
    }

    foreach ($candidate in $candidates | Select-Object -Unique) {
        if (Test-PlayerMsys2BashPath -Path $candidate) {
            return $candidate
        }
    }

    throw @"
MSYS2 was not found. Install the official MSYS2 distribution, then either:
  - keep it at the normal <system-drive>\msys64 location, or
  - set MSYS2_ROOT to the MSYS2 installation directory.

Do not place MSYS2 inside the repository or the repository-parent dependency workspace.
"@
}

function Get-PlayerMsys2Root {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$BashPath
    )

    if (-not (Test-PlayerMsys2BashPath -Path $BashPath)) {
        throw "The resolved bash executable is not an MSYS2 bash path: '$BashPath'."
    }

    $bashFullPath = [System.IO.Path]::GetFullPath($BashPath)
    $binDirectory = Split-Path -Parent $bashFullPath
    $usrDirectory = Split-Path -Parent $binDirectory
    $msysRoot = Split-Path -Parent $usrDirectory
    $expectedBash = Join-Path $msysRoot "usr/bin/bash.exe"

    if (-not (Test-Path -LiteralPath $expectedBash -PathType Leaf)) {
        throw "Unable to resolve the MSYS2 installation root from '$BashPath'."
    }

    return [System.IO.Path]::GetFullPath($msysRoot)
}

function Invoke-PlayerClang64 {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$BashPath,

        [Parameter(Mandatory = $true)]
        [string]$Command
    )

    if (-not (Test-PlayerMsys2BashPath -Path $BashPath)) {
        throw "The resolved bash executable is not an MSYS2 bash path: '$BashPath'."
    }

    $previousMsystem = $env:MSYSTEM
    $previousChereInvoking = $env:CHERE_INVOKING
    $previousPathType = $env:MSYS2_PATH_TYPE
    $temporaryScript = $null

    try {
        $env:MSYSTEM = "CLANG64"
        $env:CHERE_INVOKING = "1"
        $env:MSYS2_PATH_TYPE = "inherit"

        $msysRoot = Get-PlayerMsys2Root -BashPath $BashPath
        $msysTempDirectory = Join-Path $msysRoot "tmp"
        if (-not (Test-Path -LiteralPath $msysTempDirectory -PathType Container)) {
            [void](New-Item -ItemType Directory -Path $msysTempDirectory -Force)
        }

        $scriptName = "player-clang64-{0}.sh" -f [Guid]::NewGuid().ToString("N")
        $temporaryScript = Join-Path $msysTempDirectory $scriptName
        $msysScriptPath = "/tmp/$scriptName"

        $normalizedCommand = $Command.Replace("`r`n", "`n").Replace("`r", "`n")
        $scriptText = "set -e`n$normalizedCommand"
        if (-not $scriptText.EndsWith("`n", [System.StringComparison]::Ordinal)) {
            $scriptText += "`n"
        }

        [System.IO.File]::WriteAllText(
            $temporaryScript,
            $scriptText,
            [System.Text.UTF8Encoding]::new($false)
        )

        & $BashPath --login $msysScriptPath
        if ($LASTEXITCODE -ne 0) {
            throw "MSYS2 CLANG64 command failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        if ($temporaryScript -and (Test-Path -LiteralPath $temporaryScript -PathType Leaf)) {
            Remove-Item -LiteralPath $temporaryScript -Force
        }

        $env:MSYSTEM = $previousMsystem
        $env:CHERE_INVOKING = $previousChereInvoking
        $env:MSYS2_PATH_TYPE = $previousPathType
    }
}

Export-ModuleMember -Function @(
    "Resolve-PlayerMsys2Bash",
    "Invoke-PlayerClang64"
)
