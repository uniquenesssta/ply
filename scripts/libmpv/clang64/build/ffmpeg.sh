set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_ffmpeg_release_archive)"
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
    --enable-schannel
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
if ! grep -Eq '^#define CONFIG_SCHANNEL 1$' config.h; then
    printf 'FFmpeg configure did not enable Windows Schannel TLS support.\n' >&2
    exit 1
fi
make -j"$(nproc)"
make install
popd >/dev/null
