set -euo pipefail

CLANG64_SCRIPT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
PLAYER_PROJECT_ROOT="$(cd "$CLANG64_SCRIPT_ROOT/../../.." && pwd -P)"
PLAYER_PARENT_ROOT="$(cd "$PLAYER_PROJECT_ROOT/.." && pwd -P)"
PLAYER_DOWNLOAD_ROOT="$PLAYER_PARENT_ROOT/downloads/libmpv"
PLAYER_SOURCE_ROOT="$PLAYER_DOWNLOAD_ROOT/sources"
PLAYER_CACHE_ROOT="$PLAYER_PARENT_ROOT/cache/libmpv-build"
PLAYER_BUILD_ROOT="$PLAYER_CACHE_ROOT/build"
PLAYER_PREFIX_ROOT="$PLAYER_CACHE_ROOT/prefix"
PLAYER_METADATA_ROOT="$PLAYER_CACHE_ROOT/metadata"
PLAYER_PACKAGE_ROOT="$PLAYER_PARENT_ROOT/libmpv/${PLAYER_MPV_VERSION:?PLAYER_MPV_VERSION is required}/windows-x64"

mkdir -p \
    "$PLAYER_SOURCE_ROOT" \
    "$PLAYER_BUILD_ROOT" \
    "$PLAYER_PREFIX_ROOT" \
    "$PLAYER_METADATA_ROOT" \
    "$PLAYER_PACKAGE_ROOT"

export PATH="$PLAYER_PREFIX_ROOT/bin:$PATH"
export PKG_CONFIG_PATH="$PLAYER_PREFIX_ROOT/lib/pkgconfig:$PLAYER_PREFIX_ROOT/share/pkgconfig"
export PKG_CONFIG_LIBDIR="$PKG_CONFIG_PATH"
export CMAKE_PREFIX_PATH="$PLAYER_PREFIX_ROOT"

player_require_command() {
    local command_name="$1"
    command -v "$command_name" >/dev/null 2>&1 || {
        printf 'Required CLANG64 build tool is missing: %s\n' "$command_name" >&2
        exit 1
    }
}

player_require_build_tools() {
    local command_name
    for command_name in git clang clang++ meson ninja pkg-config nasm python make llvm-readobj; do
        player_require_command "$command_name"
    done
}

player_source_worktree_is_empty() {
    local source_dir="$1"
    [[ -z "$(find "$source_dir" -mindepth 1 -maxdepth 1 ! -name .git -print -quit)" ]]
}

player_source_status_is_only_initial_deletions() {
    local status_output="$1"
    local line

    [[ -n "$status_output" ]] || return 1

    while IFS= read -r line; do
        [[ -z "$line" ]] && continue
        [[ "${line:0:2}" == "D " ]] || return 1
    done <<< "$status_output"

    return 0
}

player_recover_incomplete_no_checkout_clone() {
    local source_dir="$1"
    local status_output="$2"

    if ! player_source_worktree_is_empty "$source_dir"; then
        return 1
    fi
    if ! player_source_status_is_only_initial_deletions "$status_output"; then
        return 1
    fi

    printf 'Recovering incomplete no-checkout source clone: %s\n' "$source_dir"
    git -C "$source_dir" reset --hard HEAD >/dev/null

    [[ -z "$(git -C "$source_dir" status --porcelain=v1 --untracked-files=all)" ]]
}

player_fetch_source() {
    local name="$1"
    local url="$2"
    local ref="$3"
    local recurse_submodules="${4:-false}"
    local expected_commit="${5:-}"
    local source_dir="$PLAYER_SOURCE_ROOT/$name"
    local status_output

    if [[ ! -d "$source_dir/.git" ]]; then
        rm -rf "$source_dir"
        git clone "$url" "$source_dir"
    fi

    status_output="$(git -C "$source_dir" status --porcelain=v1 --untracked-files=all)"
    if [[ -n "$status_output" ]]; then
        if player_recover_incomplete_no_checkout_clone "$source_dir" "$status_output"; then
            status_output="$(git -C "$source_dir" status --porcelain=v1 --untracked-files=all)"
        fi
    fi

    if [[ -n "$status_output" ]]; then
        printf 'Source checkout contains local changes and will not be overwritten: %s\n' "$source_dir" >&2
        exit 1
    fi

    git -C "$source_dir" fetch --force --tags origin "$ref"
    git -C "$source_dir" checkout --detach FETCH_HEAD

    if [[ "$recurse_submodules" == "true" ]]; then
        git -C "$source_dir" submodule sync --recursive
        git -C "$source_dir" submodule update --init --recursive
    fi

    local actual_commit
    actual_commit="$(git -C "$source_dir" rev-parse HEAD)"
    if [[ -n "$expected_commit" && "$actual_commit" != "$expected_commit" ]]; then
        printf 'Source identity mismatch for %s: expected %s, found %s\n' "$name" "$expected_commit" "$actual_commit" >&2
        exit 1
    fi

    printf '%s\n' "$actual_commit" > "$PLAYER_METADATA_ROOT/$name.commit"
    printf '%s\n' "$ref" > "$PLAYER_METADATA_ROOT/$name.ref"
    printf '%s\n' "$url" > "$PLAYER_METADATA_ROOT/$name.url"
    printf '%s\n' "$source_dir"
}

player_fresh_build_dir() {
    local name="$1"
    local build_dir="$PLAYER_BUILD_ROOT/$name"
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    printf '%s\n' "$build_dir"
}

player_meson_build_install() {
    local name="$1"
    local source_dir="$2"
    shift 2

    local build_dir
    build_dir="$(player_fresh_build_dir "$name")"

    meson setup \
        "$build_dir" \
        "$source_dir" \
        --prefix="$PLAYER_PREFIX_ROOT" \
        --bindir=bin \
        --libdir=lib \
        --includedir=include \
        --buildtype=release \
        --default-library=shared \
        --wrap-mode=nodownload \
        "$@"

    meson compile -C "$build_dir"
    meson install -C "$build_dir"
}

player_write_build_command() {
    local name="$1"
    shift
    printf '%s\n' "$*" > "$PLAYER_METADATA_ROOT/$name.build-options"
}
