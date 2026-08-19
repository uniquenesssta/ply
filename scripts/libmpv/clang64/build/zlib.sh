set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    zlib \
    "https://github.com/madler/zlib.git" \
    "${PLAYER_ZLIB_REF:?PLAYER_ZLIB_REF is required}" \
    false \
    "${PLAYER_ZLIB_COMMIT:?PLAYER_ZLIB_COMMIT is required}")"

build_dir="$(player_fresh_build_dir zlib)"
options=(
    -G Ninja
    "-DCMAKE_INSTALL_PREFIX=$PLAYER_PREFIX_ROOT"
    -DCMAKE_INSTALL_BINDIR=bin
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_BUILD_TYPE=Release
    -DZLIB_BUILD_TESTING=OFF
    -DZLIB_BUILD_SHARED=ON
    -DZLIB_BUILD_STATIC=OFF
    -DZLIB_INSTALL=ON
)
player_write_build_command zlib "${options[@]}"
cmake -S "$source_dir" -B "$build_dir" "${options[@]}"
cmake --build "$build_dir"
cmake --install "$build_dir"
