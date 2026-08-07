set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    harfbuzz \
    "https://github.com/harfbuzz/harfbuzz.git" \
    "${PLAYER_HARFBUZZ_REF:?PLAYER_HARFBUZZ_REF is required}" \
    false \
    "${PLAYER_HARFBUZZ_COMMIT:?PLAYER_HARFBUZZ_COMMIT is required}")"

options=(
    -Dglib=disabled
    -Dgobject=disabled
    -Dcairo=disabled
    -Dchafa=disabled
    -Dicu=disabled
    -Dgraphite2=disabled
    -Dfreetype=disabled
    -Dgdi=disabled
    -Ddirectwrite=disabled
    -Dcoretext=disabled
    -Dwasm=disabled
    -Dtests=disabled
    -Dintrospection=disabled
    -Ddocs=disabled
    -Dutilities=disabled
    -Dbenchmark=disabled
    -Dexperimental_api=false
)
player_write_build_command harfbuzz "${options[@]}"
player_meson_build_install harfbuzz "$source_dir" "${options[@]}"
