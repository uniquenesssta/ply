set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    libass \
    "https://github.com/libass/libass.git" \
    "${PLAYER_LIBASS_REF:?PLAYER_LIBASS_REF is required}" \
    false \
    "${PLAYER_LIBASS_COMMIT:?PLAYER_LIBASS_COMMIT is required}")"

options=(
    -Dtest=disabled
    -Dcompare=disabled
    -Dprofile=disabled
    -Dfuzz=disabled
    -Dcheckasm=disabled
    -Dfontconfig=disabled
    -Ddirectwrite=enabled
    -Dcoretext=disabled
    -Dasm=enabled
    -Dlibunibreak=disabled
    -Drequire-system-font-provider=true
)
player_write_build_command libass "${options[@]}"
player_meson_build_install libass "$source_dir" "${options[@]}"
