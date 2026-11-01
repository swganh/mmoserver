#!/usr/bin/env bash
#
# Downloads planet heightmaps into data/heightmaps/<planet>.hmpw.
#
# The archives are assets of the swganh/mmoserver v0.6.0 release. Each one is
# checked against tools/heightmaps/SHA256SUMS before it is unpacked. Every
# planet is ~150-230 MB to download and ~470 MB unpacked.
#
# Usage: tools/heightmaps/download.sh [planet ...]    (no arguments: all planets)

set -euo pipefail

base_url="https://github.com/swganh/mmoserver/releases/download/v0.6.0"
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
project_dir=$(cd "$script_dir/../.." && pwd)
checksums="$script_dir/SHA256SUMS"
dest="$project_dir/data/heightmaps"

all_planets=(corellia dantooine dathomir endor lok naboo rori talus tatooine yavin4)

# Tatooine's archive is the only one named <planet>.hmpw.zip.
archive_name() {
    if [[ $1 == tatooine ]]; then echo "tatooine.hmpw.zip"; else echo "$1.zip"; fi
}

for tool in curl sha256sum unzip; do
    if ! command -v "$tool" >/dev/null; then
        echo "error: '$tool' is required (Debian/Ubuntu: sudo apt install curl coreutils unzip)" >&2
        exit 1
    fi
done

planets=("$@")
[[ ${#planets[@]} -eq 0 ]] && planets=("${all_planets[@]}")

mkdir -p "$dest"
# Work inside the destination so the final mv is atomic: an interrupted run
# never leaves a partial .hmpw behind that a later run would skip.
tmp=$(mktemp -d "$dest/.download.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

for planet in "${planets[@]}"; do
    archive=$(archive_name "$planet")
    expected=$(awk -v f="$archive" '$2 == f { print $1 }' "$checksums")
    if [[ -z $expected ]]; then
        echo "error: unknown planet '$planet' (known: ${all_planets[*]})" >&2
        exit 1
    fi

    if [[ -f $dest/$planet.hmpw ]]; then
        echo "$planet: already present, skipping"
        continue
    fi

    echo "$planet: downloading $archive"
    curl --fail --location --proto '=https' --tlsv1.2 --retry 3 \
        --output "$tmp/$archive" "$base_url/$archive"

    actual=$(sha256sum "$tmp/$archive" | awk '{ print $1 }')
    if [[ $actual != "$expected" ]]; then
        echo "error: $archive checksum mismatch (expected $expected, got $actual)" >&2
        exit 1
    fi

    unzip -q -o "$tmp/$archive" "$planet.hmpw" -d "$tmp"
    mv "$tmp/$planet.hmpw" "$dest/$planet.hmpw"
    rm -f "$tmp/$archive"
    echo "$planet: ok"
done
