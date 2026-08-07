set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    freetype \
    "https://gitlab.freedesktop.org/freetype/freetype.git" \
    "${PLAYER_FREETYPE_REF:?PLAYER_FREETYPE_REF is required}" \
    false \
    "${PLAYER_FREETYPE_COMMIT:?PLAYER_FREETYPE_COMMIT is required}")"

options=(
    -Dbrotli=disabled
    -Dbzip2=disabled
    -Dharfbuzz=disabled
    -Dpng=disabled
    -Dtests=disabled
    -Dzlib=disabled
)
player_write_build_command freetype "${options[@]}"
player_meson_build_install freetype "$source_dir" "${options[@]}"
