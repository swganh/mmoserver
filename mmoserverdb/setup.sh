#!/usr/bin/env bash
#
# Installs the MMOServer databases into MariaDB (or MySQL) and creates game
# accounts. Replaces setup.bat on Linux. Run with -h for usage.

set -euo pipefail

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
project_dir=$(cd "$script_dir/.." && pwd)

# Settings the installer generates (database password, public address) live
# here, outside version control, and are copied into general.cfg of every
# build by apply-config (build_server.sh calls it when staging).
local_cfg="$script_dir/local.cfg"

# Databases in install order, as setup.bat installed them. The servers only
# query `swganh`; the others are kept for parity with the Windows installer.
databases=(swganh swganh_archive swganh_astromech swganh_config swganh_logs)

db_user=swganh

usage() {
    cat <<EOF
Usage: $(basename "$0") <command> [options]

Commands:
  install                 Create the databases, a '$db_user' database user with
                          a random password, and write the connection settings
                          to mmoserverdb/local.cfg and every build's
                          bin/config/general.cfg
  add-account NAME        Create a game account; asks for the password
  set-address ADDR        Change the address game clients use to reach this
                          server (same as --public-address at install)
  apply-config DIR        Copy the settings in local.cfg into DIR/general.cfg
  update-procedures       Reload the stored procedures and functions into an
                          existing database (after pulling SQL fixes); data is
                          not touched
  spawn-resources [--replace]
                          Spawn the harvestable resources (install does this).
                          With --replace, retire the current set and spawn a
                          new one; resources already harvested are kept.
                          Restart the zones to load them

Options for install:
  --force                 Drop and recreate existing MMOServer databases
                          (all characters and accounts are lost)
  --public-address ADDR   Address game clients use to reach this server
                          (default 127.0.0.1: clients on this machine only)
  --keep-test-accounts    Keep the sample accounts swganh0-9 (password: swganh)

Connecting to the database server:
  The script runs the 'mariadb' (or 'mysql') client as an administrator. On
  Ubuntu the MariaDB root account logs in through the local socket, so run
  it with sudo. For another server or account, set DB_ADMIN_USER, DB_HOST
  and DB_PORT, and MYSQL_PWD for the password.
EOF
}

die() {
    echo "error: $*" >&2
    exit 1
}

# --- Database client ----------------------------------------------------------

client=
for candidate in mariadb mysql; do
    if command -v "$candidate" >/dev/null; then
        client=$candidate
        break
    fi
done

sql() {
    [[ $client ]] || die "no MariaDB/MySQL client found (Debian/Ubuntu: sudo apt install mariadb-server)"
    local args=(--default-character-set=utf8 --batch)
    [[ ${DB_ADMIN_USER:-} ]] && args+=(--user="$DB_ADMIN_USER")
    [[ ${DB_HOST:-} ]] && args+=(--host="$DB_HOST")
    [[ ${DB_PORT:-} ]] && args+=(--port="$DB_PORT")
    [[ $session_init ]] && args+=(--init-command="$session_init")
    "$client" "${args[@]}" "$@"
}

# Session settings applied to every connection, set by check_connection.
session_init=

check_connection() {
    if ! sql -e 'SELECT 1' >/dev/null 2>&1; then
        echo "error: cannot connect to the database server as an administrator." >&2
        if [[ $EUID -ne 0 && -z ${DB_ADMIN_USER:-} ]]; then
            echo "On Ubuntu, run this script with sudo." >&2
        fi
        echo "Is MariaDB installed and running? (sudo apt install mariadb-server)" >&2
        exit 1
    fi

    # MariaDB 11.5+ defaults utf8 text to the uca1400 collations, while the
    # schema's routines use utf8_general_ci; comparing the two fails with
    # "Illegal mix of collations". Keep the collation the data was written
    # for. Older servers (no character_set_collations) already default to it.
    if sql -e 'SELECT @@character_set_collations' >/dev/null 2>&1; then
        session_init="SET SESSION character_set_collations='utf8mb3=utf8mb3_general_ci,utf8mb4=utf8mb4_general_ci'"
    fi
}

# --- local.cfg ----------------------------------------------------------------

# Sets KEY = VALUE in a config file, replacing an existing line or appending.
set_cfg() {
    local file=$1 key=$2 value=$3
    if grep -q "^[[:space:]]*$key[[:space:]]*=" "$file"; then
        sed -i "s|^[[:space:]]*$key[[:space:]]*=.*|$key = $value|" "$file"
    else
        printf '%s = %s\n' "$key" "$value" >> "$file"
    fi
}

# When run through sudo, give files back to the user who invoked it.
give_to_invoking_user() {
    if [[ ${SUDO_UID:-} && ${SUDO_GID:-} ]]; then
        chown "$SUDO_UID:$SUDO_GID" "$@"
    fi
}

apply_config() {
    local dir=$1
    [[ -f $local_cfg ]] || die "$local_cfg not found; run '$(basename "$0") install' first"
    [[ -f $dir/general.cfg ]] || die "$dir/general.cfg not found"
    local key value
    while IFS='=' read -r key value; do
        key=${key//[[:space:]]/}
        value=${value#"${value%%[![:space:]]*}"}
        [[ -z $key || $key == \#* ]] && continue
        set_cfg "$dir/general.cfg" "$key" "$value"
    done < "$local_cfg"
    chmod 600 "$dir/general.cfg"
    echo "Updated $dir/general.cfg"
}

check_address() {
    [[ $1 =~ ^[A-Za-z0-9.:-]+$ ]] || die "invalid address '$1'"
}

# The LoginServer hands clients the galaxy's address. The servers already
# listen on all interfaces; leave BindAddress at 127.0.0.1, because servers
# register under it and the ConnectionServer looks them up by the address
# their packets come from.
set_address() {
    sql -e "UPDATE swganh.galaxy SET address = '$1'"
}

apply_config_to_builds() {
    local config_dir
    for config_dir in "$project_dir"/build/*/bin/config; do
        [[ -f $config_dir/general.cfg ]] || continue
        apply_config "$config_dir"
        give_to_invoking_user "$config_dir/general.cfg"
    done
}

# --- install ------------------------------------------------------------------

import_file() {
    local database=$1 file=$2
    local output
    if ! output=$(sql ${database:+--database="$database"} < "$file" 2>&1); then
        echo "error: importing ${file#"$script_dir/"} failed:" >&2
        echo "$output" >&2
        exit 1
    fi
}

install() {
    local force= public_address=127.0.0.1 keep_test_accounts=
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --force) force=1 ;;
            --public-address)
                [[ $# -ge 2 ]] || die "--public-address needs a value"
                public_address=$2
                shift
                ;;
            --keep-test-accounts) keep_test_accounts=1 ;;
            *) die "unknown option '$1' (see -h)" ;;
        esac
        shift
    done
    check_address "$public_address"

    check_connection

    local db existing=()
    for db in "${databases[@]}"; do
        if [[ $(sql -N -e "SHOW DATABASES LIKE '$db'") ]]; then
            existing+=("$db")
        fi
    done
    if [[ ${#existing[@]} -gt 0 ]]; then
        if [[ ! $force ]]; then
            die "databases already exist: ${existing[*]}. Use --force to drop and reinstall them (all game data is lost)."
        fi
        for db in "${existing[@]}"; do
            echo "Dropping $db"
            sql -e "DROP DATABASE \`$db\`"
        done
    fi

    for db in "${databases[@]}"; do
        echo "Installing $db"
        # create.sql creates the database, so it runs without a default one.
        import_file "" "$script_dir/$db/create.sql"
        local kind file
        for kind in scripts functions procedures; do
            [[ -d $script_dir/$db/$kind ]] || continue
            while IFS= read -r file; do
                import_file "$db" "$file"
            done < <(find "$script_dir/$db/$kind" -maxdepth 1 -name '*.sql' | LC_ALL=C sort)
        done
    done

    echo "Spawning resources"
    sql -e "CALL swganh.sp_ResourceInitialSpawn()" > /dev/null

    if [[ ! $keep_test_accounts ]]; then
        echo "Removing the sample accounts swganh0-9"
        sql -e "DELETE FROM swganh.account WHERE account_username REGEXP '^swganh[0-9]\$'"
    fi

    # One database user for all servers, limited to the MMOServer databases.
    local password
    password=$(head -c 48 /dev/urandom | base64 | tr -d '/+=\n')
    password=${password:0:32}
    local host grants=""
    for host in localhost 127.0.0.1; do
        grants+="CREATE USER IF NOT EXISTS '$db_user'@'$host';"
        grants+="ALTER USER '$db_user'@'$host' IDENTIFIED BY '$password';"
        for db in "${databases[@]}"; do
            grants+="GRANT ALL PRIVILEGES ON \`$db\`.* TO '$db_user'@'$host';"
        done
    done
    sql -e "$grants FLUSH PRIVILEGES;"

    (
        umask 077
        cat > "$local_cfg" <<EOF
# Generated by mmoserverdb/setup.sh install. Not in version control.
# These settings are copied into general.cfg of each build.
DBServer = ${DB_HOST:-localhost}
DBPort = ${DB_PORT:-3306}
DBName = swganh
DBUser = $db_user
DBPass = $password
EOF
    )
    give_to_invoking_user "$local_cfg"

    set_address "$public_address"
    apply_config_to_builds

    local config_dir found=
    for config_dir in "$project_dir"/build/*/bin/config; do
        [[ -f $config_dir/general.cfg ]] || continue
        found=1
    done

    echo
    echo "Database installed. Connection settings saved in mmoserverdb/local.cfg."
    [[ $found ]] || echo "No build found yet; build_server.sh applies the settings when it stages config/."
    echo "Galaxy address: $public_address"
    echo
    echo "Next, create an account to log in with:"
    echo "    $(basename "$0") add-account <name>"
}

# --- add-account --------------------------------------------------------------

add_account() {
    [[ $# -eq 1 ]] || die "usage: $(basename "$0") add-account NAME"
    local name=$1
    [[ $name =~ ^[A-Za-z0-9_]{3,32}$ ]] || die "account names are 3-32 letters, digits or underscores"
    check_connection

    [[ -z $(sql -N -e "SELECT 1 FROM swganh.account WHERE account_username = '$name'") ]] ||
        die "account '$name' already exists"

    local password confirm
    if [[ -t 0 ]]; then
        read -r -s -p "Password for $name: " password; echo
        read -r -s -p "Repeat password: " confirm; echo
        [[ $password == "$confirm" ]] || die "passwords do not match"
    else
        IFS= read -r password
    fi
    [[ ${#password} -ge 8 ]] || die "use a password of at least 8 characters"

    # The login procedure compares SHA1(password); hex-encode the password so
    # no character in it can break out of the SQL string.
    local hex
    hex=$(printf '%s' "$password" | od -An -v -tx1 | tr -d ' \n')
    sql -e "INSERT INTO swganh.account
                (account_username, account_password, account_email, account_active)
            VALUES ('$name', SHA1(UNHEX('$hex')), '', 1);
            UPDATE swganh.account SET account_station_id = 100000000 + account_id
            WHERE account_username = '$name';"
    echo "Account '$name' created."
}

# --- update-procedures --------------------------------------------------------

# Prints "FUNCTION name" or "PROCEDURE name" for the routine a file creates.
# File names don't always match, and some names are database-qualified.
routine_in() {
    perl -0ne 'while (/^\s*CREATE\b[^(;]*?\b(FUNCTION|PROCEDURE)\s+(?:`?\w+`?\s*\.\s*)?`?(\w+)/gim) { print uc($1), " $2\n" }' "$1"
}

update_procedures() {
    local db kind file routine_kind routine_name
    for db in "${databases[@]}"; do
        for kind in functions procedures; do
            [[ -d $script_dir/$db/$kind ]] || continue
            while IFS= read -r file; do
                # Not every file drops its routine before creating it.
                while read -r routine_kind routine_name; do
                    sql --database="$db" -e "DROP $routine_kind IF EXISTS \`$routine_name\`"
                done < <(routine_in "$file")
                import_file "$db" "$file"
            done < <(find "$script_dir/$db/$kind" -maxdepth 1 -name '*.sql' | LC_ALL=C sort)
        done
    done
}

# --- spawn-resources ----------------------------------------------------------

spawn_resources() {
    local replace=
    case "${1:-}" in
        "") ;;
        --replace) replace=1 ;;
        *) die "usage: $(basename "$0") spawn-resources [--replace]" ;;
    esac
    check_connection

    # Databases installed before the spawn procedures were fixed have the
    # old versions.
    update_procedures

    local active
    active=$(sql -N -e "SELECT COUNT(*) FROM swganh.resources WHERE active = 1")
    if [[ $replace ]]; then
        sql -e "CALL swganh.sp_ResourceSpawn()" > /dev/null
    elif [[ $active -gt 0 ]]; then
        die "$active resources are already spawned. Use --replace to retire them and spawn a new set."
    else
        sql -e "CALL swganh.sp_ResourceInitialSpawn()" > /dev/null
    fi

    active=$(sql -N -e "SELECT COUNT(*) FROM swganh.resources WHERE active = 1")
    echo "$active resources spawned. Restart the zones to load them."
}

# --- main ---------------------------------------------------------------------

[[ $# -ge 1 ]] || { usage >&2; exit 2; }
command=$1
shift
case "$command" in
    install) install "$@" ;;
    add-account) add_account "$@" ;;
    set-address)
        [[ $# -eq 1 ]] || die "usage: $(basename "$0") set-address ADDR"
        check_address "$1"
        check_connection
        set_address "$1"
        echo "Galaxy address set to $1. Restart the servers to apply it."
        ;;
    apply-config)
        [[ $# -eq 1 ]] || die "usage: $(basename "$0") apply-config DIR"
        apply_config "$1"
        ;;
    update-procedures)
        [[ $# -eq 0 ]] || die "usage: $(basename "$0") update-procedures"
        check_connection
        update_procedures
        echo "Stored procedures and functions updated."
        ;;
    spawn-resources) spawn_resources "$@" ;;
    -h|--help|help) usage ;;
    *) echo "error: unknown command '$command'" >&2; usage >&2; exit 2 ;;
esac
