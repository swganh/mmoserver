#!/usr/bin/env bash
#
# Checks a Linux server for what the MMOServer portal needs, and installs it:
# a Python virtual environment, a database and database user of its own,
# and portal.cfg. Run with -h for usage. (Windows: planned, not yet.)

set -euo pipefail

portal_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
venv="$portal_dir/.venv"
cfg="$portal_dir/portal.cfg"

db_user=swganh_portal
game_db=${GAME_DB:-swganh}
portal_db=${PORTAL_DB:-swganh_portal}
port=8080
base_url=
registration=closed
admin_name=
service_user=${SUDO_USER:-$(id -un)}
assume_yes=

usage() {
    cat <<EOF
Usage: $(basename "$0") <command> [options]

Commands:
  check                   Check this server has what the portal needs. If
                          packages are missing or the folder belongs to
                          another user, offers to fix that (asks first)
  install                 check (and offer the same fixes), then: create the Python environment
                          (portal/.venv), the '$portal_db' database and a
                          '$db_user' database user, write portal/portal.cfg
                          (kept if it exists) and create the portal's tables.
                          Safe to run again, e.g. after a git pull
  create-admin NAME       Make a game account a portal Administrator; creates
                          the account if there is none (asks for a password)
  run                     Start the portal in the foreground on
                          127.0.0.1:PORT, to try it out
  service                 Install and start a systemd service that runs the
                          portal on 127.0.0.1:PORT at boot (needs sudo), and
                          print an nginx example to publish it over HTTPS

Options:
  --port N                Port the portal listens on, on 127.0.0.1 (default $port)
  --url URL               Address players open the portal at, for links in
                          emails (default http://127.0.0.1:PORT)
  --registration open|closed
                          Whether anyone can create a game account on the
                          portal (default closed: staff create them)
  --admin NAME            With install: make NAME an Administrator at the end
  --user USER             With service: the system user to run it as
                          (default $service_user)
  -y, --yes               Fix what check finds without asking (for scripts)

The game database must be installed first (mmoserverdb/setup.sh install).
Database administrator access works as for setup.sh: on Ubuntu run with
sudo; elsewhere set DB_ADMIN_USER, DB_HOST, DB_PORT and MYSQL_PWD. GAME_DB
and PORTAL_DB change the database names.
EOF
}

die() {
    echo "error: $*" >&2
    exit 1
}

# --- Output -------------------------------------------------------------------

failures=0
packages=()
family=other
fix_owner=

ok()   { printf '  \033[32mok\033[0m    %s\n' "$*"; }
warn() { printf '  \033[33mwarn\033[0m  %s\n' "$*"; }
fail() { printf '  \033[31mFAIL\033[0m  %s\n' "$*"; failures=$((failures + 1)); }
need() { packages+=("$@"); }

# --- Database client (as mmoserverdb/setup.sh) ----------------------------------

client=
for candidate in mariadb mysql; do
    if command -v "$candidate" >/dev/null; then
        client=$candidate
        break
    fi
done

sql() {
    local args=(--default-character-set=utf8mb4 --batch)
    [[ ${DB_ADMIN_USER:-} ]] && args+=(--user="$DB_ADMIN_USER")
    [[ ${DB_HOST:-} ]] && args+=(--host="$DB_HOST")
    [[ ${DB_PORT:-} ]] && args+=(--port="$DB_PORT")
    "$client" "${args[@]}" "$@"
}

# Runs a command as the user who invoked sudo, so the files it makes are
# theirs and not root's.
as_user() {
    if [[ $EUID -eq 0 && ${SUDO_USER:-} && $SUDO_USER != root ]]; then
        sudo -u "$SUDO_USER" -H "$@"
    else
        "$@"
    fi
}

# A setting in the [database] section of portal.cfg.
db_cfg_value() {
    sed -n "/^\[database\]/,/^\[/ s/^[[:space:]]*$1[[:space:]]*=[[:space:]]*//p" "$cfg" 2>/dev/null | head -n 1
}

# --- check --------------------------------------------------------------------

check() {
    local with_db=${1:-yes}
    echo "Checking this server for the portal"

    # Operating system.
    if [[ $(uname -s) != Linux ]]; then
        fail "this script is for Linux; Windows support is planned"
        return
    fi
    local os_id="" os_like="" os_name="Linux"
    if [[ -r /etc/os-release ]]; then
        os_id=$(. /etc/os-release; echo "${ID:-}")
        os_like=$(. /etc/os-release; echo "${ID_LIKE:-}")
        os_name=$(. /etc/os-release; echo "${PRETTY_NAME:-Linux}")
    fi
    ok "$os_name"
    family=other
    case " $os_id $os_like " in
        *" debian "*|*" ubuntu "*) family=debian ;;
        *" fedora "*|*" rhel "*|*" centos "*) family=fedora ;;
    esac
    [[ $family == other ]] && warn "only Debian and Ubuntu are tested; package names below may differ"

    # Python 3.10 or newer with venv.
    if ! command -v python3 >/dev/null; then
        fail "python3 not found"
        need python3 python3-venv
    elif ! python3 -c 'import sys; sys.exit(sys.version_info < (3, 10))'; then
        fail "Python $(python3 -c 'import platform; print(platform.python_version())') is too old; 3.10 or newer is needed"
    else
        ok "Python $(python3 -c 'import platform; print(platform.python_version())')"
        if python3 -c 'import venv, ensurepip' 2>/dev/null; then
            ok "Python venv and pip"
        else
            fail "Python's venv module (or ensurepip) is missing"
            need python3-venv
        fi
    fi

    # Memory: every password check takes 64 MB (Argon2id, as the LoginServer).
    local mem_mb
    mem_mb=$(awk '/^MemAvailable:/ {print int($2 / 1024)}' /proc/meminfo 2>/dev/null || echo 0)
    if (( mem_mb >= 512 )); then
        ok "${mem_mb} MB of memory available"
    elif (( mem_mb > 0 )); then
        warn "only ${mem_mb} MB of memory available; each sign-in uses 64 MB while it is checked"
    fi

    # Disk space for the virtual environment (about 60 MB).
    local free_mb
    free_mb=$(df -Pm "$portal_dir" | awk 'NR == 2 {print $4}')
    if (( free_mb >= 200 )); then
        ok "${free_mb} MB free disk space"
    else
        fail "only ${free_mb} MB free disk space; 200 MB is needed"
    fi

    # The folder: install creates .venv and portal.cfg in it as the user who
    # ran sudo, so that user must be able to write there.
    local owner=${SUDO_USER:-$(id -un)}
    if as_user test -w "$portal_dir"; then
        ok "$owner can write to $portal_dir"
    else
        fail "$owner cannot write to $portal_dir (copied with sudo?): sudo chown -R $owner: $portal_dir"
        fix_owner=$owner
    fi

    # Port.
    if command -v ss >/dev/null; then
        local listening
        listening=$(ss -ltnpH "sport = :$port" 2>/dev/null || true)
        if [[ $listening ]]; then
            # This portal itself (install.sh run, or its service) is fine.
            local pid ours=""
            for pid in $(grep -o 'pid=[0-9]*' <<< "$listening" | cut -d= -f2); do
                if tr '\0' ' ' < "/proc/$pid/cmdline" 2>/dev/null | grep -qF "$portal_dir/.venv/"; then
                    ours=1
                fi
            done
            if [[ $ours ]] || { [[ -f /etc/systemd/system/mmoportal.service ]] && systemctl is-active --quiet mmoportal 2>/dev/null; }; then
                ok "port $port is used by this portal (restart it after installing)"
            else
                fail "port $port is in use; choose another with --port"
            fi
        else
            ok "port $port is free"
        fi
    fi

    # Database client and server.
    if [[ -z $client ]]; then
        fail "no MariaDB/MySQL client (mariadb or mysql command) found"
        need mariadb-client
    else
        ok "database client: $client"
        if [[ $with_db == yes ]]; then
            check_database
        fi
    fi

    # Optional extras.
    if command -v systemctl >/dev/null && [[ -d /run/systemd/system ]]; then
        ok "systemd, for '$(basename "$0") service'"
    else
        warn "no systemd: run the portal another way (see README.md)"
    fi
    if command -v nginx >/dev/null || command -v caddy >/dev/null || command -v apache2 >/dev/null; then
        ok "a web server to publish the portal over HTTPS"
    else
        warn "no nginx, Caddy or Apache: needed to publish the portal on the internet over HTTPS (not to try it out)"
    fi

    echo
    if (( ${#packages[@]} )); then
        local unique
        unique=$(printf '%s\n' "${packages[@]}" | sort -u | tr '\n' ' ')
        case $family in
            debian) echo "Install what is missing with:"; echo "    sudo apt install $unique" ;;
            fedora) echo "Install what is missing with:"; echo "    sudo dnf install ${unique//mariadb-client/mariadb}" ;;
            *)      echo "Missing packages: $unique" ;;
        esac
        echo
    fi
    if (( failures )); then
        echo "$failures problem(s) found."
        return 1
    fi
    echo "Everything the portal needs is here."
}

check_database() {
    if ! sql -e 'SELECT 1' >/dev/null 2>&1; then
        fail "cannot connect to the database server as an administrator"
        if [[ $EUID -ne 0 && -z ${DB_ADMIN_USER:-} ]]; then
            echo "        On Ubuntu, run this script with sudo." >&2
        fi
        return
    fi
    ok "database server $(sql -N -e 'SELECT VERSION()')"

    if [[ -z $(sql -N -e "SELECT 1 FROM information_schema.TABLES WHERE TABLE_SCHEMA = '$game_db' AND TABLE_NAME = 'account'") ]]; then
        fail "no game database '$game_db' with an account table: run 'mmoserverdb/setup.sh install' first"
        return
    fi
    ok "game database '$game_db' ($(sql -N -e "SELECT COUNT(*) FROM \`$game_db\`.account") accounts)"

    local length
    length=$(sql -N -e "SELECT CHARACTER_MAXIMUM_LENGTH FROM information_schema.COLUMNS
                        WHERE TABLE_SCHEMA = '$game_db' AND TABLE_NAME = 'account' AND COLUMN_NAME = 'account_password'")
    if (( ${length:-0} >= 128 )); then
        ok "account passwords can hold Argon2id hashes"
    else
        fail "the game database is out of date (passwords too short for Argon2id): run 'mmoserverdb/setup.sh update'"
    fi
}

# --- Fixing what check found ----------------------------------------------------

# Offers to install the missing packages and give the folder to the user who
# ran sudo. Returns 0 when it fixed something (so check should run again).
offer_fixes() {
    local unique="" todo=()
    if (( ${#packages[@]} )); then
        unique=$(printf '%s\n' "${packages[@]}" | sort -u | tr '\n' ' ')
        case $family in
            debian) todo+=("install packages: $unique") ;;
            fedora) unique=${unique//mariadb-client/mariadb}; todo+=("install packages: $unique") ;;
            *) unique="" ;;   # unknown package manager: the hint is all we can do
        esac
    fi
    [[ $fix_owner ]] && todo+=("give $portal_dir to $fix_owner")
    (( ${#todo[@]} )) || return 1

    if [[ $EUID -ne 0 ]]; then
        echo "Run with sudo to let this script fix that for you."
        return 1
    fi
    if [[ ! $assume_yes ]]; then
        [[ -t 0 ]] || { echo "Run with --yes to let this script fix that."; return 1; }
        echo "This script can fix that:"
        printf '    %s\n' "${todo[@]}"
        local answer
        read -r -p "Go ahead? [Y/n] " answer
        [[ -z $answer || $answer =~ ^[Yy] ]] || return 1
    fi

    if [[ $unique ]]; then
        echo "Installing: $unique"
        case $family in
            # Word splitting of $unique is wanted: one package per word.
            debian) apt-get update -q && DEBIAN_FRONTEND=noninteractive apt-get install -y -q $unique ;;
            fedora) dnf install -y $unique ;;
        esac || die "installing the packages failed (see above)"
    fi
    if [[ $fix_owner ]]; then
        echo "Giving $portal_dir to $fix_owner"
        chown -R "$fix_owner": "$portal_dir"
    fi

    # Look again with what is there now.
    failures=0
    packages=()
    fix_owner=
    hash -r
    if [[ -z $client ]]; then
        for candidate in mariadb mysql; do
            command -v "$candidate" >/dev/null && { client=$candidate; break; }
        done
    fi
    echo
    return 0
}

# check, and if something can be fixed, offer to and check again.
check_and_fix() {
    check && return 0
    offer_fixes && check
}

# --- install ------------------------------------------------------------------

install() {
    [[ $game_db =~ ^[A-Za-z0-9_]+$ && $portal_db =~ ^[A-Za-z0-9_]+$ ]] || die "database names are letters, digits and underscores"
    check_and_fix || die "fix the problems above, then run install again"
    echo

    echo "Creating the Python environment in $venv"
    if [[ ! -x $venv/bin/python ]]; then
        as_user python3 -m venv "$venv"
    fi
    as_user "$venv/bin/pip" install --quiet --upgrade pip
    as_user "$venv/bin/pip" install --quiet -r "$portal_dir/requirements.txt"

    # Keep the database password of an earlier install, so reinstalling does
    # not break a running portal.
    local password=""
    [[ -f $cfg ]] && password=$(db_cfg_value password)
    if [[ -z $password ]]; then
        password=$(head -c 48 /dev/urandom | base64 | tr -d '/+=\n')
        password=${password:0:32}
    fi
    [[ $password =~ ^[A-Za-z0-9]+$ ]] || die "the database password in portal.cfg must be letters and digits only"

    echo "Creating the '$portal_db' database and the '$db_user' database user"
    # The portal reads and writes game accounts, reads characters, gives
    # characters the admin skills, and owns its own database; nothing else
    # in the game database.
    local host grants="CREATE DATABASE IF NOT EXISTS \`$portal_db\` CHARACTER SET utf8mb4;"
    for host in localhost 127.0.0.1; do
        grants+="CREATE USER IF NOT EXISTS '$db_user'@'$host';"
        grants+="ALTER USER '$db_user'@'$host' IDENTIFIED BY '$password';"
        grants+="GRANT ALL PRIVILEGES ON \`$portal_db\`.* TO '$db_user'@'$host';"
        grants+="GRANT SELECT, INSERT, UPDATE ON \`$game_db\`.account TO '$db_user'@'$host';"
        grants+="GRANT SELECT ON \`$game_db\`.characters TO '$db_user'@'$host';"
        # In-game admin commands: the admin skills of characters.
        grants+="GRANT SELECT, INSERT, DELETE ON \`$game_db\`.character_skills TO '$db_user'@'$host';"
        grants+="GRANT SELECT ON \`$game_db\`.skills TO '$db_user'@'$host';"
        grants+="GRANT SELECT ON \`$game_db\`.command_table TO '$db_user'@'$host';"
    done
    sql -e "$grants FLUSH PRIVILEGES;"

    if [[ ! -f $cfg ]]; then
        echo "Writing $cfg"
        local secret
        secret=$(head -c 64 /dev/urandom | base64 | tr -d '/+=\n')
        (
            umask 077
            sed -e "s|^base_url =.*|base_url = ${base_url:-http://127.0.0.1:$port}|" \
                -e "s|^secret_key =.*|secret_key = $secret|" \
                -e "s|^registration =.*|registration = $registration|" \
                -e "/^\[database\]/,/^\[/ s|^password =\$|password = $password|" \
                -e "s|^game_database =.*|game_database = $game_db|" \
                -e "s|^portal_database =.*|portal_database = $portal_db|" \
                -e "s|^host = 127.0.0.1|host = ${DB_HOST:-127.0.0.1}|" \
                -e "s|^port = 3306|port = ${DB_PORT:-3306}|" \
                "$portal_dir/portal.cfg.example" > "$cfg"
        )
        [[ ${SUDO_UID:-} ]] && chown "$SUDO_UID:$SUDO_GID" "$cfg"
    else
        echo "Keeping the settings in $cfg"
    fi

    echo "Creating the portal's tables"
    flask init-db

    if [[ $admin_name ]]; then
        flask create-admin "$admin_name"
    fi

    echo
    flask check || true
    echo
    local me="$portal_dir/install.sh"
    echo "The portal is installed, but not running yet. Next:"
    [[ $admin_name ]] || echo "    $me create-admin <your game account>"
    echo "  Try it (runs until you close the terminal or press Ctrl+C), then open http://127.0.0.1:$port:"
    echo "    $me run"
    echo "  Or run it in the background and at every boot:"
    echo "    sudo $me service"
    echo "Email for password resets is set up in $portal_dir/portal.cfg ([mail])."
}

flask() {
    [[ -x $venv/bin/flask ]] || die "the portal is not installed: run '$(basename "$0") install' first"
    (cd "$portal_dir" && as_user env PORTAL_CONFIG="$cfg" "$venv/bin/flask" --app wsgi "$@")
}

# --- run / service ------------------------------------------------------------

run() {
    [[ -x $venv/bin/gunicorn ]] || die "the portal is not installed: run '$(basename "$0") install' first"
    echo "The portal runs at http://127.0.0.1:$port (Ctrl+C stops it)"
    cd "$portal_dir"
    exec env PORTAL_CONFIG="$cfg" "$venv/bin/gunicorn" --workers 2 --bind "127.0.0.1:$port" wsgi:app
}

service() {
    [[ $EUID -eq 0 ]] || die "run 'sudo $portal_dir/install.sh service'"
    [[ -x $venv/bin/gunicorn ]] || die "the portal is not installed: run '$(basename "$0") install' first"
    command -v systemctl >/dev/null || die "systemd not found; see README.md for other ways to run the portal"
    id "$service_user" >/dev/null 2>&1 || die "no user '$service_user'"
    [[ $service_user != root ]] || die "do not run the portal as root; give a user with --user"

    local unit=/etc/systemd/system/mmoportal.service
    sed -e "s|@USER@|$service_user|g" -e "s|@DIR@|$portal_dir|g" -e "s|@PORT@|$port|g" \
        "$portal_dir/deploy/mmoportal.service.in" > "$unit"
    systemctl daemon-reload
    systemctl enable --now mmoportal
    systemctl restart mmoportal
    echo "The portal runs as the 'mmoportal' service on 127.0.0.1:$port (systemctl status mmoportal)."
    echo
    echo "To publish it, put a web server with HTTPS in front of it. For nginx,"
    echo "deploy/nginx.conf.example is a starting point; then in portal.cfg set"
    echo "base_url to the public address, secure_cookies = true and"
    echo "behind_proxy = true, and run: sudo systemctl restart mmoportal"
}

# --- Main ---------------------------------------------------------------------

[[ $# -ge 1 ]] || { usage; exit 1; }
command=$1
shift
args=()
while [[ $# -gt 0 ]]; do
    case $1 in
        --port) [[ ${2:-} =~ ^[0-9]+$ ]] || die "--port needs a number"; port=$2; shift 2 ;;
        --url) base_url=${2:?--url needs an address}; shift 2 ;;
        --registration)
            [[ ${2:-} == open || ${2:-} == closed ]] || die "--registration is open or closed"
            registration=$2; shift 2 ;;
        --admin) admin_name=${2:?--admin needs a name}; shift 2 ;;
        --user) service_user=${2:?--user needs a name}; shift 2 ;;
        -y|--yes) assume_yes=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) args+=("$1"); shift ;;
    esac
done

case $command in
    check) check_and_fix ;;
    install) install ;;
    create-admin)
        [[ ${#args[@]} -eq 1 ]] || die "usage: $(basename "$0") create-admin NAME"
        flask create-admin "${args[0]}" ;;
    run) run ;;
    service) service ;;
    -h|--help|help) usage ;;
    *) usage; exit 1 ;;
esac
