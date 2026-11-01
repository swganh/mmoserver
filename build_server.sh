#!/usr/bin/env bash
#
# Builds the MMOServer on Linux using the CMake presets in CMakePresets.json.
# Run with -h for options.

set -euo pipefail

basedir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# Ubuntu 24.04 / Debian packages for the default (system packages) build.
apt_build_tools=(build-essential cmake ninja-build)
apt_libraries=(libboost-all-dev libtbb-dev libmysqlcppconn-dev liblua5.1-0-dev
    libtolua++5.1-dev libnoise-dev libglm-dev libgtest-dev libmysqlclient-dev zlib1g-dev)
# Tools vcpkg needs to build the dependencies itself (libtirpc-dev is for libmysql).
apt_vcpkg_tools=(build-essential cmake ninja-build git curl zip unzip tar pkg-config libtirpc-dev)

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Configures and builds the servers with CMake presets, then stages config/,
script/ and heightmaps/ next to the binaries in build/<preset>/bin.

Options:
  -c                 Clean: delete the preset's build directory first
  -r                 Release build (RelWithDebInfo) instead of Debug
  -t                 Run the unit tests after building
  --vcpkg            Build the dependencies with vcpkg instead of using system
                     packages. Uses \$VCPKG_ROOT, or clones vcpkg into build/vcpkg
  --heightmaps LIST  Download heightmaps first: a comma-separated list of planets
                     or "all" (~1.8 GB download, ~4.7 GB unpacked)
  -h, --help         Show this help and exit
EOF
}

clean=
release=
run_tests=
use_vcpkg=
heightmaps=

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c) clean=1 ;;
        -r) release=1 ;;
        -t) run_tests=1 ;;
        --vcpkg) use_vcpkg=1 ;;
        --heightmaps)
            [[ $# -ge 2 ]] || { echo "error: --heightmaps needs a value" >&2; exit 2; }
            heightmaps=$2
            shift
            ;;
        -h|--help) usage; exit 0 ;;
        -[crt]?*)
            # Allow combined short flags such as -ct.
            flags=${1#-}
            shift
            set -- $(printf -- '-%s ' $(echo "$flags" | fold -w1)) "$@"
            continue
            ;;
        *) echo "error: unknown option '$1'" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

# --- Prerequisites ----------------------------------------------------------------

missing_tools=()
for tool in cmake ninja c++; do
    command -v "$tool" >/dev/null || missing_tools+=("$tool")
done

missing_packages=()
if command -v dpkg-query >/dev/null; then
    if [[ $use_vcpkg ]]; then
        wanted=("${apt_vcpkg_tools[@]}")
    else
        wanted=("${apt_build_tools[@]}" "${apt_libraries[@]}")
    fi
    [[ $heightmaps ]] && wanted+=(curl unzip)
    for package in "${wanted[@]}"; do
        if ! dpkg-query -W -f='${Status}' "$package" 2>/dev/null | grep -q "install ok installed"; then
            # Ubuntu 26.04+ dropped tolua++; CMake then builds it from source.
            if [[ $package == libtolua++5.1-dev ]] &&
                ! apt-cache policy "$package" 2>/dev/null | grep -q 'Candidate: [0-9]'; then
                continue
            fi
            missing_packages+=("$package")
        fi
    done
fi

if [[ ${#missing_packages[@]} -gt 0 ]]; then
    echo "Missing packages. Install them with:" >&2
    echo >&2
    echo "    sudo apt install ${missing_packages[*]}" >&2
    echo >&2
    exit 1
elif [[ ${#missing_tools[@]} -gt 0 ]]; then
    echo "error: required tools not found: ${missing_tools[*]}" >&2
    echo "On Debian/Ubuntu: sudo apt install ${apt_build_tools[*]} ${apt_libraries[*]}" >&2
    exit 1
fi

cmake_version=$(cmake --version | awk 'NR == 1 { print $3 }')
if ! printf '3.25\n%s\n' "$cmake_version" | sort -V -C; then
    echo "error: CMake 3.25 or newer is required for the presets (found $cmake_version)" >&2
    exit 1
fi

# --- vcpkg ------------------------------------------------------------------------

if [[ $use_vcpkg ]]; then
    if [[ -z ${VCPKG_ROOT:-} ]]; then
        export VCPKG_ROOT="$basedir/build/vcpkg"
        if [[ ! -d $VCPKG_ROOT ]]; then
            echo "Cloning vcpkg into $VCPKG_ROOT"
            git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
        fi
    fi
    if [[ ! -x $VCPKG_ROOT/vcpkg ]]; then
        "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
    fi
    preset_prefix=linux-vcpkg
else
    preset_prefix=linux
fi

if [[ $release ]]; then
    preset=$preset_prefix-release
else
    preset=$preset_prefix-debug
fi
build_dir="$basedir/build/$preset"

# --- Heightmaps -------------------------------------------------------------------

if [[ $heightmaps ]]; then
    if [[ $heightmaps == all ]]; then
        "$basedir/tools/heightmaps/download.sh"
    else
        IFS=',' read -r -a planets <<< "$heightmaps"
        "$basedir/tools/heightmaps/download.sh" "${planets[@]}"
    fi
fi

# --- Configure, build, test -------------------------------------------------------

cd "$basedir"

if [[ $clean && -d $build_dir ]]; then
    echo "Removing $build_dir"
    rm -rf "$build_dir"
fi

cmake --preset "$preset"
cmake --build --preset "$preset"

if [[ $run_tests ]]; then
    ctest --preset "$preset"
fi

# --- Stage runtime data next to the binaries ---------------------------------------

bin_dir="$build_dir/bin"
mkdir -p "$bin_dir/config" "$bin_dir/logs"

# Keep existing configs: they hold local settings such as database passwords.
for cfg in "$basedir"/data/config/*.cfg; do
    [[ -e $bin_dir/config/$(basename "$cfg") ]] || cp "$cfg" "$bin_dir/config/"
done
# Database settings written by mmoserverdb/setup.sh install.
if [[ -f $basedir/mmoserverdb/local.cfg ]]; then
    "$basedir/mmoserverdb/setup.sh" apply-config "$bin_dir/config"
fi
# Scripts are code; always refresh them.
rm -rf "$bin_dir/script"
cp -r "$basedir/data/script" "$bin_dir/script"
# Heightmaps are large; link to them instead of copying.
if [[ -d $basedir/data/heightmaps ]]; then
    ln -sfn "$basedir/data/heightmaps" "$bin_dir/heightmaps"
fi

echo
echo "Build complete: $bin_dir"
