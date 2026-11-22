# MMOServer portal

A small web application for a MMOServer community, kept apart from the game
servers. It shares one thing with them: the game database's account table.
A password set in the portal works in the game straight away, and the
other way round (both hash with libsodium's Argon2id, the same way).

**Players** sign in with their game account and can

- change their password and email address
- reset a forgotten password with a link sent by email
- see their characters
- create a game account, if the server allows it (`registration = open`)

**Staff** get an admin area, with what they can do set by role:

- find accounts by name, email or id; see their characters and history
- create accounts; change email, character limit and the active flag
- send a reset link, or set a password
- ban (for some days, or permanently) and lift bans
- give or take the in-game CSR flag
- (Administrators) make roles from permissions, and give roles to accounts
- read the audit log of every change

It is Python (Flask) with MariaDB/MySQL, and runs on Linux. Windows support
is planned.

## Installing

The game database must be installed first (`mmoserverdb/setup.sh install`,
or `setup.sh update` on an older install). Then, on the server:

    sudo portal/install.sh check

lists what is missing, with the command to install it (Debian/Ubuntu: `sudo
apt install python3 python3-venv mariadb-client`). It checks the Linux
distribution, Python 3.10 or newer with venv, memory, disk, the port, the
database client, administrator access to the database, the game database
and its version, and whether systemd and a web server are there.

    sudo portal/install.sh install --admin <your game account>

does the check, then

- creates a Python environment in `portal/.venv` with the packages in
  `requirements.txt`
- creates a `swganh_portal` database and a `swganh_portal` database user
  with a random password. That user can read and write the game's
  `account` table, read `characters`, and use its own database; nothing
  else
- writes `portal/portal.cfg` (from `portal.cfg.example`; kept if it exists)
- creates the portal's tables
- makes your game account an Administrator (or creates the account, asking
  for a password, if it does not exist)

Running `install` again is safe: do it after a `git pull` to install new
packages and tables. Options: `--port` (default 8080), `--url` (the address
players open, for links in emails), `--registration open`. Run
`portal/install.sh -h` for all of them.

As with `setup.sh`, the script reaches the database as an administrator
through the local socket on Ubuntu, so it runs with sudo; files it creates
belong to the user who ran sudo. For another database server set
`DB_ADMIN_USER`, `DB_HOST`, `DB_PORT` and `MYSQL_PWD`.

## Running

    portal/install.sh run

starts it on http://127.0.0.1:8080 to try it out.

    sudo portal/install.sh service

installs a systemd service (`mmoportal`) that starts it at boot, still on
127.0.0.1 only. To let players reach it, put a web server with HTTPS in
front of it: `deploy/nginx.conf.example` is a starting point. Then set, in
`portal.cfg`, `base_url` to the public address, `secure_cookies = true` and
`behind_proxy = true`, and `sudo systemctl restart mmoportal`. Do not open
the portal to the internet without HTTPS: passwords would cross it in clear.

## Settings

`portal.cfg` (see `portal.cfg.example` for every setting):

- `[portal]`: the site name, public address, whether players can create
  accounts, cookie and proxy settings, how long reset links last
- `[database]`: the connection, and the names of the game and portal
  databases
- `[mail]`: the SMTP server for reset links. With none, a reset link is
  written to the portal's log (`journalctl -u mmoportal`) instead, for an
  admin to pass on; staff can also set a password directly

## Permissions

Permissions are defined in `mmoportal/permissions.py`; roles are sets of
them, made by Administrators in the admin area (Roles & permissions), and
given to accounts on the account's page.

Two roles come installed: **Administrator** (every permission; the only
role that manages roles; cannot be changed or deleted, and the last one
cannot be removed) and **CSR** (look up accounts, reset passwords, ban, read
the audit log), which can be changed.

Two rules stop a permission being used to gain more: only Administrators
give roles, and only Administrators change staff accounts (any account with
a role), so a CSR cannot reset an Administrator's password.

The portal's roles are separate from the game's CSR flag
(`account.account_csr`), which gives a character the in-game admin commands;
the `accounts.game_csr` permission sets that flag.

`install.sh create-admin NAME` makes an account an Administrator from the
command line, e.g. if every Administrator is locked out.

## Security notes

- Passwords: Argon2id through libsodium (PyNaCl), with the LoginServer's
  limits. Old SHA-1 passwords are accepted and replaced at the next sign-in,
  as the LoginServer does.
- Sign-ins and reset requests are rate limited per address and per
  username. A wrong password and an unknown username answer the same.
- Reset links are random, single use, expire (60 minutes by default) and are
  stored only as a SHA-256 hash; setting a password cancels the others.
- Changing a password signs out every other session of that account.
- Every form carries a CSRF token; cookies are HttpOnly and SameSite=Lax;
  pages send a strict Content-Security-Policy.
- Bans take effect at the next game login: the portal cannot disconnect a
  player who is in game yet. The LoginServer only reads `account_banned`, so
  the portal lifts expired temporary bans itself (on page loads, at most
  every five minutes, or with `flask --app wsgi lift-expired-bans` from
  cron).

## Development

    cd portal
    .venv/bin/pip install -r requirements-dev.txt
    .venv/bin/python -m pytest tests

The tests need a MariaDB/MySQL server, where they make and drop two scratch
databases (`portal_test_game`, `portal_test_portal`). They connect as root
to 127.0.0.1:3306 by default; set `PORTAL_TEST_DB_HOST`, `_PORT`, `_USER`,
`_PASSWORD`, or `PORTAL_TEST_DB_SOCKET` (e.g. `/run/mysqld/mysqld.sock`,
with sudo). CI runs them on every change to `portal/`.

Layout:

    install.sh            check, install, run, service (Linux)
    wsgi.py               entry point for gunicorn and the flask command
    schema/NNN_*.sql      the portal database, applied in order by init-db
    mmoportal/
      __init__.py         the application
      config.py           reads portal.cfg
      db.py               database access ({game} and {portal} in SQL)
      passwords.py        hashing, compatible with the LoginServer
      permissions.py      the permission list and checks
      auth_state.py       sessions, CSRF, rate limits, audit log
      accounts.py         account changes shared by the pages
      views/              auth (sign in, register, reset), account, admin
      templates/, static/

A new feature is a new blueprint in `views/` with its templates, its
permissions added to `PERMISSIONS`, and a new `schema/` file for any tables.

## Ideas for later

- An in-game item shop (portal currency or rewards delivered to a
  character)
- Reading and sending in-game mail from the portal
- A skill and profession calculator (from the skill data the servers load)
- Character tools for staff: find, inspect, move, restore
- Server status and live control (players online, broadcast, kick), which
  needs an admin channel into the running servers (docs/ROADMAP.md)
- Windows support in the install script
