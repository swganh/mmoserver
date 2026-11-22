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
- give or take the in-game CSR flag, and give characters the in-game admin
  commands (In-game CSR page; see below)
- (Administrators) make roles from permissions, and give roles to accounts
- read the audit log of every change

It is Python (Flask) with MariaDB/MySQL, and runs on Linux. Windows support
is planned.

## Installing

The game database must be installed first (`mmoserverdb/setup.sh install`,
or `setup.sh update` on an older install). Then, on the server:

    sudo portal/install.sh check

lists what is missing, with the command to install it (Debian/Ubuntu: `sudo
apt install python3 python3-venv mariadb-client`), and offers to install it
for you (apt on Debian/Ubuntu, dnf on Fedora). It also offers to give the
folder to your user if it belongs to root, e.g. after copying it with sudo.
Add `--yes` to do that without asking. It checks the Linux
distribution, Python 3.10 or newer with venv, memory, disk, the port, the
database client, administrator access to the database, the game database
and its version, and whether systemd and a web server are there.

    sudo portal/install.sh install --admin <your game account>

does the check, then

- creates a Python environment in `portal/.venv` with the packages in
  `requirements.txt`
- creates a `swganh_portal` database and a `swganh_portal` database user
  with a random password. That user can read and write the game's
  `account` table, read `characters`, `skills` and `command_table`, add and
  remove rows of `character_skills` (the admin skills), and use its own
  database; nothing else
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

### In-game staff tools

The In-game CSR page lists what staff can do inside the game today. The
servers have two kinds of staff power:

- the account's **CSR flag**: the CSR menu on Character Builder terminals
  (blue frog, experience, credits, items, professions, teleport...) and
  logging in while the galaxy is locked
- the hidden **admin profession**, whose skills unlock the admin slash
  commands that have server code: `/broadcast`, `/broadcastPlanet`,
  `/broadcastGalaxy`, `/shutdownGalaxy`, `/cancelShutdownGalaxy`,
  `/admin_warp_self` and `/admin_sys_msg`. Nothing gives these skills in
  game; staff with the `game.admin_commands` permission give them per
  character on the account's page (rows in `character_skills`)

The game client limits what can be typed, by its own command table
(`datatables/command/command_table.iff`): only the three broadcasts are
client commands, marked for god level 5, and the other four are not in it,
so the client does not send them. Where the client takes a character's god
level from is still being tested; the CSR flag's value is sent to it, so it
may be that.

Both apply from the character's next login. The page also lists the 129
other admin commands the client knows that the servers do not carry out
yet. Installs from before this page need `install.sh install` run again,
which lets the portal's database user change character skills.

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
