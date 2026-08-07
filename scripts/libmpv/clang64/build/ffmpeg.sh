set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    ffmpeg \
    "https://git.ffmpeg.org/ffmpeg.git" \
    "${PLAYER_FFMPEG_REF:?PLAYER_FFMPEG_REF is required}" \
    false \
    "${PLAYER_FFMPEG_COMMIT:?PLAYER_FFMPEG_COMMIT is required}")"
build_dir="$(player_fresh_build_dir ffmpeg)"

options=(
    --prefix="$PLAYER_PREFIX_ROOT"
    --bindir="$PLAYER_PREFIX_ROOT/bin"
    --libdir="$PLAYER_PREFIX_ROOT/lib"
    --incdir="$PLAYER_PREFIX_ROOT/include"
    --arch=x86_64
    --target-os=mingw32
    --cc=clang
    --cxx=clang++
    --ar=llvm-ar
    --ranlib=llvm-ranlib
    --strip=llvm-strip
    --pkg-config=pkg-config
    --disable-autodetect
    --disable-gpl
    --disable-nonfree
    --enable-shared
    --disable-static
    --disable-programs
    --disable-doc
    --disable-debug
    --disable-avdevice
)
player_write_build_command ffmpeg "${options[@]}"

pushd "$build_dir" >/dev/null
"$source_dir/configure" "${options[@]}"
make -j"$(nproc)"
make install
popd >/dev/null
