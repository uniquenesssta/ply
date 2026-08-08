Set-StrictMode -Version Latest

function Get-PlayerQtRuntimeRequiredFiles {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
        [string]$Preset
    )

    if ($Preset -eq "windows-msvc-debug") {
        return @(
            "Qt6Cored.dll",
            "Qt6Guid.dll",
            "Qt6Qmld.dll",
            "Qt6Quickd.dll",
            "platforms/qwindowsd.dll"
        )
    }

    return @(
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Qml.dll",
        "Qt6Quick.dll",
        "platforms/qwindows.dll"
    )
}

function Assert-PlayerQtRuntimeDeployment {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
        [string]$Preset,

        [Parameter(Mandatory = $true)]
        [string]$OutputDirectory
    )

    $missingFiles = [System.Collections.Generic.List[string]]::new()
    foreach ($relativePath in Get-PlayerQtRuntimeRequiredFiles -Preset $Preset) {
        $fullPath = Join-Path $OutputDirectory $relativePath
        if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
            [void]$missingFiles.Add($relativePath)
        }
    }

    if ($missingFiles.Count -gt 0) {
        $missingList = $missingFiles -join ", "
        throw "Qt runtime deployment is incomplete in '$OutputDirectory'. Missing: $missingList"
    }
}

function Invoke-PlayerQtRuntimeDeployment {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
        [string]$Preset,

        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot,

        [Parameter(Mandatory = $true)]
        [string]$QtRoot
    )

    $projectRootPath = (Resolve-Path -LiteralPath $ProjectRoot).Path
    $qtRootPath = (Resolve-Path -LiteralPath $QtRoot).Path
    $outputDirectory = Join-Path $projectRootPath "build/$Preset"
    $executablePath = Join-Path $outputDirectory "Player.exe"
    $qmlSourceDirectory = Join-Path $projectRootPath "src/presentation/qml"
    $windeployQt = Join-Path $qtRootPath "bin/windeployqt.exe"

    foreach ($requiredPath in @(
        @("Player executable", $executablePath, "Leaf"),
        @("QML source directory", $qmlSourceDirectory, "Container"),
        @("windeployqt", $windeployQt, "Leaf")
    )) {
        if (-not (Test-Path -LiteralPath $requiredPath[1] -PathType $requiredPath[2])) {
            throw "$($requiredPath[0]) was not found at '$($requiredPath[1])'."
        }
    }

    $configurationFlag = if ($Preset -eq "windows-msvc-debug") { "--debug" } else { "--release" }
    $arguments = @(
        $configurationFlag,
        "--force",
        "--verbose", "0",
        "--no-translations",
        "--qmldir", $qmlSourceDirectory,
        "--dir", $outputDirectory,
        $executablePath
    )

    & $windeployQt @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed with exit code $LASTEXITCODE."
    }

    Assert-PlayerQtRuntimeDeployment -Preset $Preset -OutputDirectory $outputDirectory
    Write-Host "[OK] Qt runtime deployed for $Preset -> build/$Preset"
}

Export-ModuleMember -Function @(
    "Assert-PlayerQtRuntimeDeployment",
    "Invoke-PlayerQtRuntimeDeployment"
)
