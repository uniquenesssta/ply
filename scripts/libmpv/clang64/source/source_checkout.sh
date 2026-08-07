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

player_clone_source_repository() {
    local name="$1"
    local url="$2"
    local source_dir="$3"

    if [[ -e "$source_dir" ]]; then
        if player_source_directory_is_empty "$source_dir"; then
            rmdir "$source_dir" || {
                printf 'Unable to remove empty source directory before clone: %s\n' "$source_dir" >&2
                return 1
            }
        else
            printf 'Source path exists but is not a Git checkout and will not be overwritten: %s\n' "$source_dir" >&2
            return 1
        fi
    fi

    local max_attempts=3
    local attempt
    local exit_code=1
    local clone_dir

    for ((attempt = 1; attempt <= max_attempts; attempt++)); do
        clone_dir="$PLAYER_SOURCE_ROOT/.${name}.clone.${BASHPID}.${RANDOM}.${attempt}"
        if [[ -e "$clone_dir" ]]; then
            printf 'Generated clone staging path already exists and will not be overwritten: %s\n' "$clone_dir" >&2
            return 1
        fi

        if git clone "$url" "$clone_dir" >&2; then
            if [[ -e "$source_dir" ]]; then
                printf 'Source path appeared while clone was running and will not be overwritten: %s\n' "$source_dir" >&2
                rm -rf -- "$clone_dir"
                return 1
            fi
            if ! mv -- "$clone_dir" "$source_dir"; then
                printf 'Unable to move completed clone into source directory: %s\n' "$source_dir" >&2
                rm -rf -- "$clone_dir"
                return 1
            fi
            return 0
        else
            exit_code=$?
        fi

        rm -rf -- "$clone_dir"
        if ((attempt < max_attempts)); then
            printf 'Cloning %s failed (attempt %d/%d); retrying in %d seconds...\n' \
                "$name" "$attempt" "$max_attempts" "$((attempt * 2))" >&2
            sleep "$((attempt * 2))"
        fi
    done

    printf 'Cloning %s failed after %d attempts; no source checkout was installed.\n' \
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

    # This function is consumed through command substitution. Keep stdout reserved
    # for the final source path and send all operational diagnostics to stderr.
    if [[ ! -d "$source_dir/.git" ]]; then
        if ! player_clone_source_repository "$name" "$url" "$source_dir"; then
            return 1
        fi
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

    if ! player_git_network_retry \
        "Fetching $name source ref $ref" \
        git -C "$source_dir" fetch --force --tags origin "$ref"; then
        return 1
    fi
    if ! git -C "$source_dir" checkout --detach FETCH_HEAD >&2; then
        printf 'Unable to checkout fetched source ref for %s: %s\n' "$name" "$ref" >&2
        return 1
    fi

    if [[ "$recurse_submodules" == "true" ]]; then
        if ! git -C "$source_dir" submodule sync --recursive >&2; then
            printf 'Unable to synchronize submodule configuration for %s.\n' "$name" >&2
            return 1
        fi
        if ! player_git_network_retry \
            "Updating $name source submodules" \
            git -C "$source_dir" submodule update --init --recursive; then
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
