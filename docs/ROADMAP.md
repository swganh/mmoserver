# Roadmap

Where the revival of MMOServer (pre-CU SWG server) stands and where it is
going. It extends the plan in section 6 of `docs/CODE_REVIEW_2026.md`; open
bugs are in `docs/KNOWN_ISSUES.md`, and the scripting plan is in
`docs/SCRIPTING.md`.

## Vision

- **Communities run Linux** dedicated servers: one machine (or VM) with
  MariaDB and the servers, set up with `build_server.sh`,
  `mmoserverdb/setup.sh` and `server.sh`.
- **Developers work on Windows**, building and debugging the servers in
  Visual Studio against a database on a Linux VM or a remote host.
- **The C++ core provides mechanics; content lives in Lua scripts** that
  can be changed without rebuilding (`docs/SCRIPTING.md`). Everything stays
  GPL.
- **Tools for the people running a server**: a management portal for
  admins, QA and community leaders, and a load-testing tool to know how many
  players a server can take.

## Content sources

Pre-CU content (creature spawns, NPCs, quests, the tutorial) is recreated
from the client's own files (datatables, string files, object templates,
sounds) and from public material from the time: player guides, creature
databases, wiki pages, recordings. The leaked SOE server source is not used,
for reference or otherwise: it was never licensed, and anything derived from
it puts the project and every community hosting it at legal risk.

## Prior work: swganh

[anhstudios/swganh](https://github.com/anhstudios/swganh) (2011-2016) was
the same team's ground-up rewrite: a C++11 core of services (login,
connection, chat, simulation, combat, travel, terrain, ...) with Python 3
scripting, MIT licensed. It never reached MMOServer's gameplay coverage, and
MMOServer's `src/anh` already carries parts of it (brought in by the 2015
"Backports" commit). Useful as a reference:

- **What it scripted:** commands, radial menus, attributes, buffs, spawns,
  weather and character creation are Python, the rest is C++ services. A
  concrete list for the inventory in `docs/SCRIPTING.md`.
- **Object templates generated from client data:** about 16,300 generated
  template files, one per client object template; the content-from-client-
  files approach above, done once already.
- **Forum login plugins** (phpBB, SMF) for accounts, relevant to the
  community tools below.

Rules for using it: ideas and architecture are free to learn from. Code can
be used under its MIT licence, keeping its copyright notice. Leave out any
piece that encodes server-side knowledge which could not have come from the
client or public material, as for the leaked source.

## Status

Tested end to end on Ubuntu 26.04 with MariaDB 11.8 and a real client:
galaxy online, account login, character creation, the full tutorial to
Tatooine, tutorial skip, character switching after a disconnect, surveying
and sampling, resource rotation while running. The servers run unattended
overnight and recover from database connection timeouts. What is confirmed
and what still needs a client test is in `docs/TEST_PLAN.md`.

| Phase | State |
|---|---|
| 1. Reproducible build | Linux done (merged). Windows build next |
| 2. Runnable server | Installer, launcher and runtime fixes done (PRs #2 and #3 on the fork); client tests in `docs/TEST_PLAN.md`; security and remote database access to do |
| 3. Refactoring: scripts first, then modernisation | Planned (`docs/SCRIPTING.md`) |
| 4. Content | Planned |
| 5. Pre-CU gameplay | Not started |
| Tools: portal, load testing | Planned (below) |

## Next steps

1. **Client tests** of the remaining fixes (`docs/TEST_PLAN.md`), fixing
   whatever fails.
2. **Windows build** (phase 1, remaining), with the remote database setup
   from phase 2 so Windows development can use the Linux VM's database.
3. **Refactoring, scripts first** (phase 3): move tutorial and other
   content out of C++ into Lua behind a documented script API, with event
   hooks instead of polling; then dependency upgrades and the rest of the
   modernisation.

## Phase 1: reproducible build

Done for Linux: CMake with imported targets, presets, vcpkg manifest with
Lua 5.1 and tolua++ overlay ports, `build_server.sh`, verified heightmap
downloads, CI on Ubuntu 24.04 and 26.04 plus the vcpkg route.

**Remaining: Windows.**
- `windows-x64` debug and release presets using vcpkg (the manifest and
  overlay ports are already platform-neutral).
- Rewrite `build_server.bat` as a thin wrapper like `build_server.sh`;
  remove `deps/build_deps.bat` and `tools/windows/postbuild.bat`.
- Heightmaps with `curl.exe` and the same SHA-256 checks.
- A Windows job in CI.
- `server.ps1` (or the existing `.bat` launchers updated) to start and stop
  the servers like `server.sh`.

## Phase 2: runnable server for a community

Done: `mmoserverdb/setup.sh` (install, accounts, galaxy address, resource
spawning, reloading procedures), SQL repairs, `server.sh` (start, stop,
status, debug sessions in tmux with gdb), and the runtime fixes listed in
`docs/KNOWN_ISSUES.md`.

Remaining:
- **Remote database access** for the Windows development setup:
  `setup.sh allow-host <address>` to create the `swganh` user for one
  development machine and print the MariaDB `bind-address` change. Never
  open the database port to the internet; document an SSH tunnel as the
  alternative.
- **Security** (code review section 4, before any public server):
  bounds-checked `Message` reads; player text out of fixed SQL buffers and
  into prepared statements; hashed passwords (Argon2id or bcrypt) with
  re-hash on next login; servers binding only to `BindAddress` instead of
  every interface (`KNOWN_ISSUES.md` #3); unknown usernames no longer
  creating accounts by default (#3a).
- **Character limits**: the per-account limit is `account_characters_allowed`
  (default 2), changed by hand in SQL today. `setup.sh` options for the
  default limit of new accounts and for one account's limit, so test servers
  can be generous; later the same in the management portal.
- **Docker Compose** stack (MariaDB plus servers) as an alternative to the
  manual install.
- **Operations**: database backup and restore commands in `setup.sh`, log
  rotation, and running the servers as systemd services for unattended
  servers.

## Phase 3: refactoring, scripts first

Once Linux and Windows both build. Two parts, in this order.

**Scripts first** (`docs/SCRIPTING.md`): inventory of what is in Lua versus
C++ (with swganh's split as a reference), event hooks instead of polling, a
small documented script API, the tutorial moved onto it and its C++
special cases (38 `isTutorial()` checks, hard-coded NPC and terminal ids)
removed, then the same for other content; bindings that build on both
platforms (tolua++ built as part of the build, or sol2), Lua staying 5.1.

**Then modernisation and robustness:**
- **Dependency upgrades**: current Boost, oneTBB, Connector/C++; replace
  `boost::thread`/`mutex`/`future` with `std::`. Lua stays 5.1.
- **Warnings to zero** with `-Wall -Wextra` / `/W4`, and ASan/UBSan builds
  in CI.
- **Tests**: a ZoneServer unit test target (its `main` lives in
  `ZoneServer.cpp`, which the test layout expects in `main.cpp`), tests for
  the mechanics fixed during testing (health and death, posture, spawning,
  visibility), and a fuzz harness for `Message` parsing.
- **Combat and creature AI review**: the creature state machines are
  largely stripped (`AttackableCreature`, `LairObject`): lairs never spawn,
  creatures barely react, and players stay in combat after their opponent
  dies (`KNOWN_ISSUES.md` 5f, 5i, 5j).
- **Memory**: a loop in Loot all ran a zone out of memory (#10). Keep memory
  monitoring as standard: LeakSanitizer in CI, the memory log during client
  tests, and memory per zone in the server metrics.
- **Refactoring where the bugs were**: object creation and destruction
  sent to clients, the spatial grid (its bucket maths has negative-coordinate
  and size quirks; positions inside buildings are mirrored), the database
  layer (prepared statements throughout), and creature AI (largely
  missing).

## Phase 4: content

On the script API from phase 3: research pre-CU creature spawns, lairs,
NPCs and quests from the sources above and write them as scripts and data;
generating object data from the client's files (as swganh did for its
templates).

## Phase 5: pre-CU gameplay

From the code review: combat first (the heart of pre-CU and thin today),
then factions and the GCW, loot, guilds, vendors and the bazaar, vehicles
and pets, then Jedi. Crafting and the entertainer systems show what a
complete system looks like in this codebase.

## Tools

### Management portal

For admins, QA and community leaders; a web application, reachable only
from trusted networks or behind a login, never open by default.

- **Read and edit data** (works against the database directly): accounts
  (create, ban, reset password, permissions, character limit), characters (find, inspect,
  move, restore), galaxy settings (address, lock to staff, settings now in
  `config_server` such as the resource rotation interval), resources
  (current spawn, force a rotation).
- **Live server control** (needs an admin channel into the running
  servers): status and players per zone, broadcast messages, kick, shut down
  with a countdown, reload scripts. The servers have no such channel today
  beyond console keys (`q` to quit, `l` to lock the galaxy in the
  ConnectionServer). Options: a small authenticated HTTP/JSON endpoint on
  the ConnectionServer bound to localhost, or an admin message type on the
  existing cluster protocol relayed by a portal backend.
- **QA helpers**: issue reports from in game (with position and
  character), test-character presets, and server metrics (below).
- Existing in-game admin and CSR commands should be inventoried first; the
  portal can reuse what they already do.

### Load and stress testing

- A **headless bot client** that speaks the SWG protocol: session setup,
  login, galaxy and character selection, zoning in, then scripted
  behaviour (move, chat, travel, fight, log out and back in). The servers'
  own `NetworkManager` implements the protocol, so building the bot on it is
  the quickest route.
- Run hundreds of bots from another machine, ramping up, to find limits
  per zone and per galaxy.
- **Server-side metrics** to read alongside: players per zone, zone tick
  duration, database job queue depth, message queue sizes, memory. Exposed
  in the logs first, then in the portal.
- The same bots make **regression tests** for the bugs found in manual
  testing: reconnect and character switching, link-dead cleanup, zone
  transfers, idle disconnects.
