set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    libplacebo \
    "https://code.videolan.org/videolan/libplacebo.git" \
    "${PLAYER_LIBPLACEBO_REF:?PLAYER_LIBPLACEBO_REF is required}" \
    true \
    "${PLAYER_LIBPLACEBO_COMMIT:?PLAYER_LIBPLACEBO_COMMIT is required}")"

options=(
    -Dvulkan=disabled
    -Dopengl=enabled
    -Dd3d11=disabled
    -Dglslang=disabled
    -Dshaderc=disabled
    -Dlcms=disabled
    -Ddovi=disabled
    -Dlibdovi=disabled
    -Ddemos=false
    -Dtests=false
    -Dbench=false
    -Dfuzz=false
    -Dunwind=disabled
    -Dxxhash=disabled
)
player_write_build_command libplacebo "${options[@]}"
player_meson_build_install libplacebo "$source_dir" "${options[@]}"
