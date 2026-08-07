set -euo pipefail

: "${PLAYER_SOURCE_ROOT:?PLAYER_SOURCE_ROOT is required}"
: "${PLAYER_METADATA_ROOT:?PLAYER_METADATA_ROOT is required}"

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

player_source_directory_is_empty() {
    local source_dir="$1"
    [[ -d "$source_dir" ]] || return 1
    [[ -z "$(find "$source_dir" -mindepth 1 -maxdepth 1 -print -quit)" ]]
}

player_git_http() {
    git -c http.version=HTTP/1.1 "$@"
}

player_git_network_retry() {
    local description="$1"
    shift

    local max_attempts=3
    local attempt
    local exit_code=1

    for ((attempt = 1; attempt <= max_attempts; attempt++)); do
        if "$@" >&2; then
            return 0
        else
            exit_code=$?
        fi

        if ((attempt < max_attempts)); then
            printf '%s failed (attempt %d/%d); retrying in %d seconds...\n' \
                "$description" "$attempt" "$max_attempts" "$((attempt * 2))" >&2
            sleep "$((attempt * 2))"
        fi
    done

    printf '%s failed after %d attempts.\n' "$description" "$max_attempts" >&2
    return "$exit_code"
}

player_prepare_source_destination() {
    local source_dir="$1"

    if [[ ! -e "$source_dir" ]]; then
        return 0
    fi
    if player_source_directory_is_empty "$source_dir"; then
        rmdir "$source_dir" || {
            printf 'Unable to remove empty source directory before checkout: %s\n' "$source_dir" >&2
            return 1
        }
        return 0
    fi

    printf 'Source path exists but is not a Git checkout and will not be overwritten: %s\n' "$source_dir" >&2
    return 1
}

player_install_shallow_source_repository() {
    local name="$1"
    local url="$2"
    local ref="$3"
    local expected_commit="$4"
    local source_dir="$5"

    if ! player_prepare_source_destination "$source_dir"; then
        return 1
    fi

    local max_attempts=3
    local attempt
    local exit_code=1
    local checkout_dir
    local actual_commit

    for ((attempt = 1; attempt <= max_attempts; attempt++)); do
        checkout_dir="$PLAYER_SOURCE_ROOT/.${name}.checkout.${BASHPID}.${RANDOM}.${attempt}"
        if [[ -e "$checkout_dir" ]]; then
            printf 'Generated checkout staging path already exists and will not be overwritten: %s\n' "$checkout_dir" >&2
            return 1
        fi

        if ! git init "$checkout_dir" >&2 || \
           ! git -C "$checkout_dir" remote add origin "$url" >&2; then
            rm -rf -- "$checkout_dir"
            printf 'Unable to initialize source checkout staging repository for %s.\n' "$name" >&2
            return 1
        fi

        if player_git_http -C "$checkout_dir" fetch --depth=1 --no-tags origin "$ref" >&2; then
            if ! git -C "$checkout_dir" checkout --detach FETCH_HEAD >&2; then
                rm -rf -- "$checkout_dir"
                printf 'Unable to checkout fetched source ref for %s: %s\n' "$name" "$ref" >&2
                return 1
            fi
            if ! actual_commit="$(git -C "$checkout_dir" rev-parse HEAD)"; then
                rm -rf -- "$checkout_dir"
                printf 'Unable to resolve initial source commit for %s.\n' "$name" >&2
                return 1
            fi
            if [[ -n "$expected_commit" && "$actual_commit" != "$expected_commit" ]]; then
                rm -rf -- "$checkout_dir"
                printf 'Source identity mismatch for %s: expected %s, found %s\n' \
                    "$name" "$expected_commit" "$actual_commit" >&2
                return 1
            fi
            if [[ -e "$source_dir" ]]; then
                rm -rf -- "$checkout_dir"
                printf 'Source path appeared while checkout was running and will not be overwritten: %s\n' "$source_dir" >&2
                return 1
            fi
            if ! mv -- "$checkout_dir" "$source_dir"; then
                rm -rf -- "$checkout_dir"
                printf 'Unable to move completed checkout into source directory: %s\n' "$source_dir" >&2
                return 1
            fi
            return 0
        else
            exit_code=$?
        fi

        rm -rf -- "$checkout_dir"
        if ((attempt < max_attempts)); then
            printf 'Fetching initial %s source failed (attempt %d/%d); retrying in %d seconds...\n' \
                "$name" "$attempt" "$max_attempts" "$((attempt * 2))" >&2
            sleep "$((attempt * 2))"
        fi
    done

    printf 'Fetching initial %s source failed after %d attempts; no source checkout was installed.\n' \
        "$name" "$max_attempts" >&2
    return "$exit_code"
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

    printf 'Recovering incomplete no-checkout source clone: %s\n' "$source_dir" >&2
    if ! git -C "$source_dir" reset --hard HEAD >&2; then
        printf 'Unable to recover incomplete no-checkout source clone: %s\n' "$source_dir" >&2
        return 1
    fi

    local recovered_status
    if ! recovered_status="$(git -C "$source_dir" status --porcelain=v1 --untracked-files=all)"; then
        printf 'Unable to verify recovered source checkout: %s\n' "$source_dir" >&2
        return 1
    fi
    [[ -z "$recovered_status" ]]
}

player_write_source_metadata() {
    local name="$1"
    local url="$2"
    local ref="$3"
    local actual_commit="$4"

    if ! printf '%s\n' "$actual_commit" > "$PLAYER_METADATA_ROOT/$name.commit" || \
       ! printf '%s\n' "$ref" > "$PLAYER_METADATA_ROOT/$name.ref" || \
       ! printf '%s\n' "$url" > "$PLAYER_METADATA_ROOT/$name.url"; then
        printf 'Unable to write source metadata for %s.\n' "$name" >&2
        return 1
    fi
}

player_fetch_source() {
    local name="$1"
    local url="$2"
    local ref="$3"
    local recurse_submodules="${4:-false}"
    local expected_commit="${5:-}"
    local source_dir="$PLAYER_SOURCE_ROOT/$name"
    local status_output
    local installed_new=false

    # This function is consumed through command substitution. Keep stdout reserved
    # for the final source path and send all operational diagnostics to stderr.
    if [[ ! -d "$source_dir/.git" ]]; then
        if ! player_install_shallow_source_repository \
            "$name" "$url" "$ref" "$expected_commit" "$source_dir"; then
            return 1
        fi
        installed_new=true
    fi

    if ! status_output="$(git -C "$source_dir" status --porcelain=v1 --untracked-files=all)"; then
        printf 'Unable to inspect source checkout: %s\n' "$source_dir" >&2
        return 1
    fi
    if [[ -n "$status_output" ]]; then
        if player_recover_incomplete_no_checkout_clone "$source_dir" "$status_output"; then
            if ! status_output="$(git -C "$source_dir" status --porcelain=v1 --untracked-files=all)"; then
                printf 'Unable to inspect recovered source checkout: %s\n' "$source_dir" >&2
                return 1
            fi
        fi
    fi

    if [[ -n "$status_output" ]]; then
        printf 'Source checkout contains local changes and will not be overwritten: %s\n' "$source_dir" >&2
        return 1
    fi

    if [[ "$installed_new" != "true" ]]; then
        if ! player_git_network_retry \
            "Fetching $name source ref $ref" \
            player_git_http -C "$source_dir" fetch --force --depth=1 --no-tags origin "$ref"; then
            return 1
        fi
        if ! git -C "$source_dir" checkout --detach FETCH_HEAD >&2; then
            printf 'Unable to checkout fetched source ref for %s: %s\n' "$name" "$ref" >&2
            return 1
        fi
    fi

    if [[ "$recurse_submodules" == "true" ]]; then
        if ! git -C "$source_dir" submodule sync --recursive >&2; then
            printf 'Unable to synchronize submodule configuration for %s.\n' "$name" >&2
            return 1
        fi
        if ! player_git_network_retry \
            "Updating $name source submodules" \
            player_git_http -C "$source_dir" submodule update --init --recursive --depth 1 --jobs 1; then
            return 1
        fi
    fi

    local actual_commit
    if ! actual_commit="$(git -C "$source_dir" rev-parse HEAD)"; then
        printf 'Unable to resolve checked-out source commit for %s.\n' "$name" >&2
        return 1
    fi
    if [[ -n "$expected_commit" && "$actual_commit" != "$expected_commit" ]]; then
        printf 'Source identity mismatch for %s: expected %s, found %s\n' "$name" "$expected_commit" "$actual_commit" >&2
        return 1
    fi

    if ! player_write_source_metadata "$name" "$url" "$ref" "$actual_commit"; then
        return 1
    fi

    printf '%s\n' "$source_dir"
}
