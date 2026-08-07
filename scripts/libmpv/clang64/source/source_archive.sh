set -euo pipefail

: "${PLAYER_DOWNLOAD_ROOT:?PLAYER_DOWNLOAD_ROOT is required}"
: "${PLAYER_CACHE_ROOT:?PLAYER_CACHE_ROOT is required}"
: "${PLAYER_METADATA_ROOT:?PLAYER_METADATA_ROOT is required}"

PLAYER_ARCHIVE_ROOT="$PLAYER_DOWNLOAD_ROOT/archives"
PLAYER_ARCHIVE_SOURCE_ROOT="$PLAYER_CACHE_ROOT/archive-sources"
PLAYER_SOURCE_TRUST_ROOT="$PLAYER_CACHE_ROOT/source-trust"

mkdir -p "$PLAYER_ARCHIVE_ROOT" "$PLAYER_ARCHIVE_SOURCE_ROOT" "$PLAYER_SOURCE_TRUST_ROOT"

player_require_archive_tools() {
    local command_name
    for command_name in curl gpg tar xz git; do
        command -v "$command_name" >/dev/null 2>&1 || {
            printf 'Required source-archive tool is missing: %s\n' "$command_name" >&2
            return 1
        }
    done
}

player_download_archive_file() {
    local url="$1"
    local destination="$2"
    local part_path="${destination}.part"

    if [[ -f "$destination" ]]; then
        return 0
    fi
    if [[ -e "$destination" && ! -f "$destination" ]]; then
        printf 'Archive destination exists but is not a file: %s\n' "$destination" >&2
        return 1
    fi

    if ! curl \
        --fail \
        --location \
        --retry 5 \
        --retry-all-errors \
        --retry-delay 2 \
        --connect-timeout 30 \
        --continue-at - \
        --output "$part_path" \
        "$url"; then
        printf 'Archive download failed and resumable partial data was preserved: %s\n' "$part_path" >&2
        return 1
    fi

    if ! mv -- "$part_path" "$destination"; then
        printf 'Unable to finalize downloaded archive file: %s\n' "$destination" >&2
        return 1
    fi
}

player_verify_signing_key() {
    local key_path="$1"
    local expected_fingerprint="$2"
    local gnupg_home="$3"

    rm -rf -- "$gnupg_home"
    mkdir -p "$gnupg_home"
    chmod 700 "$gnupg_home"

    if ! gpg --batch --homedir "$gnupg_home" --import "$key_path" >&2; then
        printf 'Unable to import source-release signing key: %s\n' "$key_path" >&2
        return 1
    fi

    local fingerprint_output
    if ! fingerprint_output="$(gpg --batch --homedir "$gnupg_home" --with-colons --fingerprint)"; then
        printf 'Unable to inspect imported source-release signing key.\n' >&2
        return 1
    fi
    if ! grep -Fq "fpr:::::::::${expected_fingerprint}:" <<< "$fingerprint_output"; then
        printf 'Source-release signing-key fingerprint mismatch. Expected %s.\n' \
            "$expected_fingerprint" >&2
        return 1
    fi
}

player_verify_remote_tag_commit() {
    local repository_url="$1"
    local ref="$2"
    local expected_commit="$3"
    local max_attempts=3
    local attempt
    local remote_output=""
    local exit_code=1

    for ((attempt = 1; attempt <= max_attempts; attempt++)); do
        if remote_output="$(player_git_http ls-remote "$repository_url" "refs/tags/${ref}^{}")"; then
            break
        else
            exit_code=$?
        fi

        if ((attempt < max_attempts)); then
            printf 'Resolving remote source tag %s failed (attempt %d/%d); retrying in %d seconds...\n' \
                "$ref" "$attempt" "$max_attempts" "$((attempt * 2))" >&2
            sleep "$((attempt * 2))"
        fi
    done

    if [[ -z "$remote_output" ]]; then
        printf 'Unable to resolve peeled commit for remote source tag %s after %d attempts.\n' \
            "$ref" "$max_attempts" >&2
        return "$exit_code"
    fi

    local remote_commit
    remote_commit="$(printf '%s\n' "$remote_output" | awk 'NF >= 2 { print $1; exit }')"
    if [[ -z "$remote_commit" ]]; then
        printf 'Unable to parse peeled commit for remote source tag: %s\n' "$ref" >&2
        return 1
    fi
    if [[ "$remote_commit" != "$expected_commit" ]]; then
        printf 'Remote source tag identity mismatch for %s: expected %s, found %s\n' \
            "$ref" "$expected_commit" "$remote_commit" >&2
        return 1
    fi
}

player_extract_verified_tar_xz() {
    local archive_path="$1"
    local source_name="$2"
    local expected_top_level="$3"
    local destination="$PLAYER_ARCHIVE_SOURCE_ROOT/$source_name"
    local staging="$PLAYER_ARCHIVE_SOURCE_ROOT/.${source_name}.extract.${BASHPID}.${RANDOM}"

    rm -rf -- "$destination" "$staging"
    mkdir -p "$staging"

    if ! tar -xJf "$archive_path" -C "$staging"; then
        rm -rf -- "$staging"
        printf 'Unable to extract verified source archive: %s\n' "$archive_path" >&2
        return 1
    fi

    local extracted_root="$staging/$expected_top_level"
    if [[ ! -d "$extracted_root" ]]; then
        rm -rf -- "$staging"
        printf 'Verified source archive did not contain expected root directory: %s\n' \
            "$expected_top_level" >&2
        return 1
    fi

    if ! mv -- "$extracted_root" "$destination"; then
        rm -rf -- "$staging" "$destination"
        printf 'Unable to install extracted source tree: %s\n' "$destination" >&2
        return 1
    fi
    rm -rf -- "$staging"
    printf '%s\n' "$destination"
}

player_fetch_ffmpeg_release_archive() {
    player_require_archive_tools

    local version="${PLAYER_FFMPEG_VERSION:?PLAYER_FFMPEG_VERSION is required}"
    local ref="${PLAYER_FFMPEG_REF:?PLAYER_FFMPEG_REF is required}"
    local commit="${PLAYER_FFMPEG_COMMIT:?PLAYER_FFMPEG_COMMIT is required}"
    local archive_url="${PLAYER_FFMPEG_ARCHIVE_URL:?PLAYER_FFMPEG_ARCHIVE_URL is required}"
    local signature_url="${PLAYER_FFMPEG_ARCHIVE_SIGNATURE_URL:?PLAYER_FFMPEG_ARCHIVE_SIGNATURE_URL is required}"
    local signing_key_url="${PLAYER_FFMPEG_SIGNING_KEY_URL:?PLAYER_FFMPEG_SIGNING_KEY_URL is required}"
    local signing_fingerprint="${PLAYER_FFMPEG_SIGNING_FINGERPRINT:?PLAYER_FFMPEG_SIGNING_FINGERPRINT is required}"
    local repository_url="https://git.ffmpeg.org/ffmpeg.git"
    local archive_path="$PLAYER_ARCHIVE_ROOT/ffmpeg-${version}.tar.xz"
    local signature_path="${archive_path}.asc"
    local signing_key_path="$PLAYER_ARCHIVE_ROOT/ffmpeg-release-signing-key.asc"
    local gnupg_home="$PLAYER_SOURCE_TRUST_ROOT/ffmpeg-gnupg"

    player_download_archive_file "$archive_url" "$archive_path" >&2
    player_download_archive_file "$signature_url" "$signature_path" >&2
    player_download_archive_file "$signing_key_url" "$signing_key_path" >&2

    player_verify_signing_key "$signing_key_path" "$signing_fingerprint" "$gnupg_home"
    if ! gpg --batch --homedir "$gnupg_home" --verify "$signature_path" "$archive_path" >&2; then
        printf 'FFmpeg release archive signature verification failed. Existing downloaded files were not overwritten.\n' >&2
        return 1
    fi

    player_verify_remote_tag_commit "$repository_url" "$ref" "$commit"

    printf '%s\n' "$commit" > "$PLAYER_METADATA_ROOT/ffmpeg.commit"
    printf '%s\n' "$ref" > "$PLAYER_METADATA_ROOT/ffmpeg.ref"
    printf '%s\n' "$archive_url" > "$PLAYER_METADATA_ROOT/ffmpeg.url"
    printf '%s\n' "official-signed-release-archive" > "$PLAYER_METADATA_ROOT/ffmpeg.source-kind"
    printf '%s\n' "$signing_fingerprint" > "$PLAYER_METADATA_ROOT/ffmpeg.signing-fingerprint"

    player_extract_verified_tar_xz "$archive_path" ffmpeg "ffmpeg-${version}"
}
