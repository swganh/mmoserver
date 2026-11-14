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
  allow-host ADDR         Let the '$db_user' database user connect from another
                          machine, such as a Windows development PC
                          (docs/WINDOWS.md); prints the remaining steps
  deny-host ADDR          Undo allow-host
  update                  Bring an existing database up to date after a git
                          pull: update-procedures, then migrate
  update-procedures       Reload the stored procedures and functions into an
                          existing database (after pulling SQL fixes); data is
                          not touched
  migrate                 Apply the data changes in mmoserverdb/migrations that
                          this database does not have yet (install applies
                          them all)
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

    echo "Applying migrations"
    migrate

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

    # An Argon2id hash from the LoginServer (LoginServer --hash-password). It
    # is made of letters, digits and $,=+/ only, so it is safe in the SQL.
    # The newest build's; none at all is fine (an unmatched glob stays as is).
    local login_server="" candidate stored=""
    for candidate in "$project_dir"/build/*/bin/LoginServer; do
        if [[ -x $candidate && ( -z $login_server || $candidate -nt $login_server ) ]]; then
            login_server=$candidate
        fi
    done
    if [[ -n $login_server ]]; then
        # A LoginServer older than --hash-password would start up instead:
        # the timeout stops it, and its output is not a hash.
        stored=$(printf '%s\n' "$password" | timeout 30 "$login_server" --hash-password 2>/dev/null) || stored=""
        [[ $stored =~ ^\$argon2id\$[A-Za-z0-9\$,=+/]+$ ]] || stored=""
    fi

    local value
    if [[ -n $stored ]]; then
        value="'$stored'"
    else
        # No LoginServer built: the old SHA-1 form, which the LoginServer
        # replaces with an Argon2id hash at the account's first login. The
        # password is hex-encoded so no character in it can break the SQL.
        echo "note: no LoginServer build that hashes passwords; the password is stored as SHA-1 until the first login" >&2
        local hex
        hex=$(printf '%s' "$password" | od -An -v -tx1 | tr -d ' \n')
        value="SHA1(UNHEX('$hex'))"
    fi

    sql -e "INSERT INTO swganh.account
                (account_username, account_password, account_email, account_active)
            VALUES ('$name', $value, '', 1);
            UPDATE swganh.account SET account_station_id = 100000000 + account_id
            WHERE account_username = '$name';"
    echo "Account '$name' created."
}

# --- allow-host / deny-host ---------------------------------------------------

local_cfg_value() {
    [[ -f $local_cfg ]] || die "$local_cfg not found; run '$(basename "$0") install' first"
    sed -n "s/^[[:space:]]*$1[[:space:]]*=[[:space:]]*//p" "$local_cfg" | tail -n 1
}

check_host() {
    # An address, a host name, or a MariaDB pattern such as 192.168.1.%.
    [[ $1 =~ ^[A-Za-z0-9.:%_-]+$ ]] || die "invalid host '$1'"
    [[ $1 == % ]] && die "refusing to allow every host; name the development machine"
    case $1 in localhost|127.0.0.1) die "'$1' is set up by install already" ;; esac
    return 0
}

allow_host() {
    [[ $# -eq 1 ]] || die "usage: $(basename "$0") allow-host ADDR"
    local host=$1
    check_host "$host"
    check_connection
    local password
    password=$(local_cfg_value DBPass)
    [[ $password ]] || die "no DBPass in $local_cfg"

    # Same user and password as the local servers, so the development
    # machine uses the same general.cfg settings apart from DBServer.
    local grants="CREATE USER IF NOT EXISTS '$db_user'@'$host';"
    grants+="ALTER USER '$db_user'@'$host' IDENTIFIED BY '$password';"
    local db
    for db in "${databases[@]}"; do
        grants+="GRANT ALL PRIVILEGES ON \`$db\`.* TO '$db_user'@'$host';"
    done
    sql -e "$grants FLUSH PRIVILEGES;"
    echo "Database user '$db_user' may now connect from $host."

    local bind port
    bind=$(sql -N -e "SELECT @@GLOBAL.bind_address" 2>/dev/null || true)
    port=$(local_cfg_value DBPort)
    port=${port:-3306}
    echo
    echo "Remaining steps on this machine:"
    if [[ -z $bind || $bind == 127.0.0.1 || $bind == localhost || $bind == ::1 ]]; then
        echo "  - MariaDB only listens on this machine (bind-address = ${bind:-127.0.0.1})."
        echo "    Add this machine's LAN address, keeping 127.0.0.1 for the local servers"
        echo "    (MariaDB 10.11 and later take a list), in"
        echo "    /etc/mysql/mariadb.conf.d/50-server.cnf:"
        echo "      bind-address = 127.0.0.1,<this machine's LAN address>"
        echo "    then: sudo systemctl restart mariadb"
    else
        echo "  - MariaDB listens on $bind: nothing to change."
    fi
    if command -v ufw >/dev/null && LC_ALL=C ufw status 2>/dev/null | grep -q '^Status: active'; then
        echo "  - Open the port to $host only:"
        echo "      sudo ufw allow from ${host//%/0/24} to any port $port proto tcp"
    fi
    echo "  Never open port $port to the internet. Over an untrusted network, use an"
    echo "  SSH tunnel instead (docs/WINDOWS.md)."
    echo
    echo "On the development machine, set in build\\<preset>\\bin\\config\\general.cfg:"
    echo "  DBServer = <this machine's address>"
    echo "  DBPort = $port"
    echo "  DBUser = $(local_cfg_value DBUser)"
    echo "  DBPass = (the DBPass in mmoserverdb/local.cfg on this machine)"
}

deny_host() {
    [[ $# -eq 1 ]] || die "usage: $(basename "$0") deny-host ADDR"
    check_host "$1"
    check_connection
    sql -e "DROP USER IF EXISTS '$db_user'@'$1'; FLUSH PRIVILEGES;"
    echo "Database user '$db_user' may no longer connect from $1."
}

# --- migrate --------------------------------------------------------------------

# Migrations are numbered SQL files, mmoserverdb/migrations/NNN_name.sql, run
# once each in number order with swganh as the default database. Each one
# applied is recorded in swganh_config.swganh_db_version. A migration must be
# safe to run on a database that already has its change (fresh installs import
# the original data first, then every migration).
migrate() {
    local dir=$script_dir/migrations file name applied count=0
    [[ -d $dir ]] || return 0
    while IFS= read -r file; do
        name=$(basename "$file")
        [[ $name =~ ^[0-9]{3}_[a-z0-9_]+\.sql$ ]] || die "bad migration name '$name' (NNN_lowercase_name.sql)"
        applied=$(sql -N -e "SELECT COUNT(*) FROM swganh_config.swganh_db_version WHERE scriptName = '$name'")
        [[ $applied -eq 0 ]] || continue
        echo "Applying migration $name"
        import_file swganh "$file"
        sql -e "INSERT INTO swganh_config.swganh_db_version (revisionMajor, revisionMinor, scriptName, dateApplied)
                VALUES ('${name%%_*}', '0', '$name', NOW())"
        count=$((count + 1))
    done < <(find "$dir" -maxdepth 1 -name '*.sql' | LC_ALL=C sort)
    echo "$count migration(s) applied."
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
    migrate)
        [[ $# -eq 0 ]] || die "usage: $(basename "$0") migrate"
        check_connection
        migrate
        ;;
    update)
        [[ $# -eq 0 ]] || die "usage: $(basename "$0") update"
        check_connection
        update_procedures
        echo "Stored procedures and functions updated."
        migrate
        ;;
    allow-host) allow_host "$@" ;;
    deny-host) deny_host "$@" ;;
    -h|--help|help) usage ;;
    *) echo "error: unknown command '$command'" >&2; usage >&2; exit 2 ;;
esac
