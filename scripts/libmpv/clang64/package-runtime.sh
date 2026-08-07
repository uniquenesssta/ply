set -euo pipefail
source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)/common.sh"

player_require_build_tools

rm -rf \
    "$PLAYER_PACKAGE_ROOT/include" \
    "$PLAYER_PACKAGE_ROOT/lib" \
    "$PLAYER_PACKAGE_ROOT/bin" \
    "$PLAYER_PACKAGE_ROOT/licenses" \
    "$PLAYER_PACKAGE_ROOT/dependency-manifest.json"
mkdir -p \
    "$PLAYER_PACKAGE_ROOT/include" \
    "$PLAYER_PACKAGE_ROOT/lib" \
    "$PLAYER_PACKAGE_ROOT/bin" \
    "$PLAYER_PACKAGE_ROOT/licenses"

if [[ ! -d "$PLAYER_PREFIX_ROOT/include/mpv" ]]; then
    printf 'mpv headers were not installed under %s/include/mpv\n' "$PLAYER_PREFIX_ROOT" >&2
    exit 1
fi
cp -a "$PLAYER_PREFIX_ROOT/include/mpv" "$PLAYER_PACKAGE_ROOT/include/"

shopt -s nullglob
prefix_dlls=("$PLAYER_PREFIX_ROOT/bin/"*.dll)
if (( ${#prefix_dlls[@]} == 0 )); then
    printf 'No runtime DLLs were produced under %s/bin\n' "$PLAYER_PREFIX_ROOT" >&2
    exit 1
fi
cp -a "${prefix_dlls[@]}" "$PLAYER_PACKAGE_ROOT/bin/"

find_packaged_dll() {
    local name="$1"
    find "$PLAYER_PACKAGE_ROOT/bin" -maxdepth 1 -type f -iname "$name" -print -quit
}

find_runtime_candidate() {
    local name="$1"
    local candidate
    candidate="$(find "$PLAYER_PREFIX_ROOT/bin" -maxdepth 1 -type f -iname "$name" -print -quit)"
    if [[ -n "$candidate" ]]; then
        printf '%s\n' "$candidate"
        return
    fi

    candidate="$(find /clang64/bin -maxdepth 1 -type f -iname "$name" -print -quit 2>/dev/null || true)"
    if [[ -n "$candidate" ]]; then
        printf '%s\n' "$candidate"
    fi
}

declare -a scan_queue=("$PLAYER_PACKAGE_ROOT/bin/"*.dll)
declare -A scanned=()
queue_index=0
while (( queue_index < ${#scan_queue[@]} )); do
    current="${scan_queue[$queue_index]}"
    queue_index=$((queue_index + 1))

    current_key="$(basename "$current" | tr '[:upper:]' '[:lower:]')"
    if [[ -n "${scanned[$current_key]:-}" ]]; then
        continue
    fi
    scanned[$current_key]=1

    while IFS= read -r dependency_name; do
        [[ -z "$dependency_name" ]] && continue
        if [[ -n "$(find_packaged_dll "$dependency_name")" ]]; then
            continue
        fi

        dependency_source="$(find_runtime_candidate "$dependency_name")"
        if [[ -z "$dependency_source" ]]; then
            continue
        fi

        cp -a "$dependency_source" "$PLAYER_PACKAGE_ROOT/bin/"
        scan_queue+=("$PLAYER_PACKAGE_ROOT/bin/$(basename "$dependency_source")")
    done < <(
        llvm-readobj --coff-imports "$current" \
            | sed -n 's/^[[:space:]]*Name: \(.*\.[dD][lL][lL]\)$/\1/p'
    )
done

mapfile -t mpv_runtime_candidates < <(
    find "$PLAYER_PACKAGE_ROOT/bin" -maxdepth 1 -type f \
        \( -iname 'libmpv-2.dll' -o -iname 'mpv-2.dll' \) \
        -print
)
if (( ${#mpv_runtime_candidates[@]} != 1 )); then
    printf 'Expected exactly one libmpv runtime DLL, found %d\n' "${#mpv_runtime_candidates[@]}" >&2
    printf '%s\n' "${mpv_runtime_candidates[@]}" >&2
    exit 1
fi
mpv_runtime="${mpv_runtime_candidates[0]}"

{
    printf 'LIBRARY %s\n' "$(basename "$mpv_runtime")"
    printf 'EXPORTS\n'
    llvm-readobj --coff-exports "$mpv_runtime" \
        | sed -n 's/^[[:space:]]*Name: \(mpv_[A-Za-z0-9_]*\)$/\1/p' \
        | sort -u
} > "$PLAYER_PACKAGE_ROOT/lib/mpv.def"

export_count="$(grep -c '^mpv_' "$PLAYER_PACKAGE_ROOT/lib/mpv.def" || true)"
if (( export_count == 0 )); then
    printf 'No mpv_* exports were discovered in %s\n' "$mpv_runtime" >&2
    exit 1
fi
printf '%s\n' "$(basename "$mpv_runtime")" > "$PLAYER_METADATA_ROOT/mpv.runtime-name"

copy_license() {
    local destination_name="$1"
    shift
    local candidate
    for candidate in "$@"; do
        if [[ -f "$candidate" ]]; then
            mkdir -p "$PLAYER_PACKAGE_ROOT/licenses/$destination_name"
            cp -a "$candidate" "$PLAYER_PACKAGE_ROOT/licenses/$destination_name/"
        fi
    done
}

copy_license mpv \
    "$PLAYER_SOURCE_ROOT/mpv/LICENSE.LGPL" \
    "$PLAYER_SOURCE_ROOT/mpv/Copyright"
copy_license ffmpeg \
    "$PLAYER_SOURCE_ROOT/ffmpeg/COPYING.LGPLv2.1" \
    "$PLAYER_SOURCE_ROOT/ffmpeg/COPYING.LGPLv3"
copy_license libplacebo "$PLAYER_SOURCE_ROOT/libplacebo/LICENSE"
copy_license libass "$PLAYER_SOURCE_ROOT/libass/COPYING"
copy_license freetype \
    "$PLAYER_SOURCE_ROOT/freetype/LICENSE.TXT" \
    "$PLAYER_SOURCE_ROOT/freetype/docs/FTL.TXT"
copy_license fribidi "$PLAYER_SOURCE_ROOT/fribidi/COPYING"
copy_license harfbuzz "$PLAYER_SOURCE_ROOT/harfbuzz/COPYING"

printf 'Staged libmpv runtime package at %s\n' "$PLAYER_PACKAGE_ROOT"
