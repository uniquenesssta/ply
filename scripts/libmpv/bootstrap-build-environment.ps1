$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$msysModulePath = Join-Path $PSScriptRoot "modules/Msys2Environment.psm1"
Import-Module $msysModulePath -Force

Push-Location $projectRoot
try {
    $bash = Resolve-PlayerMsys2Bash

    $packages = @(
        "base-devel",
        "git",
        "curl",
        "gnupg",
        "tar",
        "xz",
        "mingw-w64-clang-x86_64-toolchain",
        "mingw-w64-clang-x86_64-cmake",
        "mingw-w64-clang-x86_64-meson",
        "mingw-w64-clang-x86_64-ninja",
        "mingw-w64-clang-x86_64-pkgconf",
        "mingw-w64-clang-x86_64-nasm",
        "mingw-w64-clang-x86_64-python"
    )

    Write-Host "MSYS2 bash: $bash"
    Write-Host "Installing/verifying CLANG64 build-only packages..."
    $packageArgs = $packages -join " "
    Invoke-PlayerClang64 -BashPath $bash -Command "pacman -S --needed --noconfirm $packageArgs"

    $requiredCommands = @(
        "git",
        "curl",
        "gpg",
        "tar",
        "xz",
        "clang",
        "clang++",
        "cmake",
        "meson",
        "ninja",
        "pkg-config",
        "nasm",
        "python",
        "make",
        "llvm-readobj"
    )

    $quotedCommands = ($requiredCommands | ForEach-Object { "'$_'" }) -join " "
    Invoke-PlayerClang64 -BashPath $bash -Command @"
set -e
for command_name in $quotedCommands; do
    command -v "`$command_name" >/dev/null 2>&1 || {
        echo "Missing required CLANG64 build tool: `$command_name" >&2
        exit 1
    }
done
printf '%s\n' 'MSYS2 CLANG64 build environment is ready.'
"@
}
finally {
    Pop-Location
}
