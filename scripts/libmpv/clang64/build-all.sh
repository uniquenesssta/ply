set -euo pipefail
SCRIPT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
source "$SCRIPT_ROOT/common.sh"

player_require_build_tools

rm -rf "$PLAYER_BUILD_ROOT" "$PLAYER_PREFIX_ROOT" "$PLAYER_METADATA_ROOT"
mkdir -p "$PLAYER_BUILD_ROOT" "$PLAYER_PREFIX_ROOT" "$PLAYER_METADATA_ROOT"

build_steps=(
    freetype
    fribidi
    harfbuzz
    libass
    libplacebo
    ffmpeg
    mpv
)

for step in "${build_steps[@]}"; do
    printf '\n=== Building %s ===\n' "$step"
    bash "$SCRIPT_ROOT/build/$step.sh"
done

printf '\n=== Packaging runtime ===\n'
bash "$SCRIPT_ROOT/package-runtime.sh"
printf '\nlibmpv source build and runtime staging completed.\n'
