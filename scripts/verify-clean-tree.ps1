$ErrorActionPreference = "Stop"

$changes = git status --porcelain
if ($changes) {
    Write-Error "The working tree is not clean:`n$changes"
    exit 1
}

Write-Host "Working tree is clean."
