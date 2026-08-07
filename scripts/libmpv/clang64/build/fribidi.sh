set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    fribidi \
    "https://github.com/fribidi/fribidi.git" \
    "${PLAYER_FRIBIDI_REF:?PLAYER_FRIBIDI_REF is required}" \
    false \
    "${PLAYER_FRIBIDI_COMMIT:?PLAYER_FRIBIDI_COMMIT is required}")"

options=(
    -Ddeprecated=true
    -Ddocs=false
    -Dbin=false
    -Dtests=false
)
player_write_build_command fribidi "${options[@]}"
player_meson_build_install fribidi "$source_dir" "${options[@]}"
