$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force

Push-Location $projectRoot
try {
    $stagedFiles = @(git diff --cached --name-only --diff-filter=ACMR)
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to inspect the Git staging area."
    }

    $forbiddenPatterns = @(
        '(^|/)(Qt|qt|libmpv|downloads|cache)(/|$)',
        '(^|/)(build|out|dist)(/|$)'
    )
    $forbiddenStagedFiles = @(
        $stagedFiles | Where-Object {
            $path = ($_ -replace '\\', '/')
            $forbiddenPatterns | Where-Object { $path -match $_ }
        }
    )
    if ($forbiddenStagedFiles.Count -gt 0) {
        throw "Third-party SDK, download, cache, or build files are staged:`n$($forbiddenStagedFiles -join [Environment]::NewLine)"
    }

    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    foreach ($entry in $layout.PSObject.Properties) {
        Assert-PlayerRepositoryRelativePath -Name $entry.Name -Path ([string]$entry.Value)
    }

    $changes = git status --porcelain
    if ($LASTEXITCODE -ne 0) {
        throw "git status failed."
    }
    if ($changes) {
        throw "The working tree is not clean:`n$changes"
    }

    Write-Host "Working tree is clean; all committed dependency/tool locations are repository-relative."
}
finally {
    Pop-Location
}
