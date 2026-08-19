set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)/common.sh"

player_require_build_tools
source_dir="$(player_fetch_source \
    mpv \
    "https://github.com/mpv-player/mpv.git" \
    "${PLAYER_MPV_TAG:?PLAYER_MPV_TAG is required}" \
    false \
    "${PLAYER_MPV_COMMIT:?PLAYER_MPV_COMMIT is required}")"

options=(
    -Dgpl=false
    -Dcplayer=false
    -Dlibmpv=true
    -Dbuild-date=false
    -Dtests=false
    -Dfuzzers=false
    -Dcdda=disabled
    -Dcplugins=disabled
    -Ddvbin=disabled
    -Ddvdnav=disabled
    -Diconv=disabled
    -Djavascript=disabled
    -Djpeg=disabled
    -Dlcms2=disabled
    -Dlibarchive=disabled
    -Dlibavdevice=disabled
    -Dlibbluray=disabled
    -Dlua=disabled
    -Drubberband=disabled
    -Duchardet=disabled
    -Dvapoursynth=disabled
    -Dzimg=disabled
    -Dzlib=enabled
    -Dwasapi=enabled
    -Dplain-gl=enabled
    -Dgl=enabled
    -Dvulkan=disabled
    -Degl=disabled
    -Dd3d11=disabled
    -Ddirect3d=disabled
    -Dgl-win32=disabled
    -Dhtml-build=disabled
    -Dmanpage-build=disabled
    -Dpdf-build=disabled
)
player_write_build_command mpv "${options[@]}"
player_meson_build_install mpv "$source_dir" "${options[@]}"
