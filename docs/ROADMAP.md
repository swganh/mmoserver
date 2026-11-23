# Roadmap

Where the revival of MMOServer (pre-CU SWG server) stands and where it is
going. It extends the plan in section 6 of `docs/CODE_REVIEW_2026.md`; what
exists against the pre-CU game is in `docs/PRECU_AUDIT.md`; open
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
| 1. Reproducible build | Linux done (merged). Windows in progress (`windows-dev-env`) |
| 2. Runnable server | Installer, launcher and runtime fixes done (PRs #2 and #3 on the fork); client tests in `docs/TEST_PLAN.md`; security and remote database access to do |
| 3. Refactoring: scripts first, then modernisation | Planned (`docs/SCRIPTING.md`) |
| 4. Content | Planned |
| 5. Pre-CU gameplay | Combat in progress: data-driven rules (`data/script/combat/rules.lua`), specials, brawler moves, incapacitation and cloning, wounds, bleeding and battle fatigue, medic and entertainer healing; creature AI (roam, aggro, fight back, leash); lairs spawn (`docs/COMBAT.md`) |
| Tools: portal, load testing | Portal started: accounts, password resets, staff roles and permissions (`portal/`). The rest of the portal and load testing planned (below) |

## Next steps

**This list is what we work on, in this order.** Something new that comes
up goes under "Parked" below with a line on why, not into the current
work. The order comes from `docs/PRECU_AUDIT.md`, which says what exists
against the pre-CU game; update both when a step is done.

Done (Nov 2026): creature tables from both Prima guides (#17 on the fork),
scout harvesting (#18), specials (#19), security (#20), player search, chat
and friends (#21).

1. **Bazaar pass** (in progress, `bazaar` branch): listing and buying
   confirmed; next, a listing's details, retrieving and cancelling, then
   bidding. Then the pull request.
2. **Test pass of what is built but never played** (the audit's "Code"
   rows; tests T27-T40 in `docs/TEST_PLAN.md`): crafting and
   experimentation, harvesters and factories, housing and permissions,
   mission terminals, groups, vehicles, image design. Each confirmed or
   turned into issues; it may find working systems before any
   new work starts.
3. **Fill the world**: creature spawns and lairs on every planet (today:
   around Bestine only). Exercises the AI and combat already built. The
   client has no spawn or lair tables (those were the server's), so the
   sources are: the 2004 guide's creature tables, already transcribed (683
   creatures and NPCs, each with its planet), and its planet chapters (pp.
   118-159) for where they live; the client's `clientregion` tables (city
   areas, to keep spawns out), `clientpoi` (points of interest) and
   `object/mobile/` templates (which creatures it can draw).
4. **Factions and the Galactic Civil War**: joining, rank and recruiters,
   overt and covert, TEF, faction kills and points. Needed by PvP, the
   theme parks and the pilot professions.
5. **Finish the combat professions**: players' worn armour (2004 guide,
   pp. 168-169), encumbrance, the remaining states, poison and disease, the
   medic review (stimpacks, `KNOWN_ISSUES.md` #25), the elites and cones;
   then the professions with commands but no effects (squad leader,
   commando, bounty hunter with bounty missions, smuggler, ranger).
6. **Pets**: creature handler (taming, commands), droids, creature mounts,
   bio-engineer.
7. **Communities**: guilds, player vendors and merchants, player cities
   and politicians.
8. **Content** (phase 4, on the script API from phase 3): quests, theme
   parks, points of interest, dungeons.
9. **Jedi**, last, as in the game.

Refactoring, scripts first (phase 3), comes in where a step needs it:
content (step 8) waits on the script API.

### Parked

Worth doing, not now. Pick one up only when the list above reaches it, or
when it unblocks the current step.

| What | Why parked | Groundwork done |
|---|---|---|
| **Jump to Lightspeed** (space, ships, pilots, shipwright) | Needs factions (step 4) and pets (step 6); about as large as the ground game so far (`docs/PRECU_AUDIT.md`) | `tools/client/jtl_inventory.py` lists the expansion's content in a client; `docs/JTL.md` has the Publish 14 client's (16 space zones, 313 ship chassis, 1,115 components, 320 schematics, 752 space quests). Still to do: document the ship messages, try a ZoneServer on a space scene |
| Login rate limiting | Before a public server, not needed on a test server | |
| Per-planet MOTD | Cosmetic | |
| Sampling into a full container (#26) | Rare, small | |
| T24 leftovers (waypoint rename, bank tip), T25 clone data | Tests of the security work, low risk | |
| Management portal (beyond accounts), load testing (Tools, below) | After the gameplay steps | `portal/`: accounts, resets, roles and permissions, audit log, Linux installer |
| A long run on a rented cloud VM (Azure, AWS or similar) | The test VM froze once overnight with memory, swap and CPU flat, put down to the host PC (KNOWN_ISSUES #40); a cloud VM would rule the host out, and is a step towards a public server | |
| Housing set-up and test steps | Record how to place, sign, name and test each house type once Tatooine is populated; signs are tuned so far for the Tatooine large and small houses and the guild halls (KNOWN_ISSUES #30) | |
| Deleting test characters in bulk | The test server has about 40 test characters; the character select screen's Delete removes them properly, one at a time. A `tools/` script through the same deletion procedure would do a list | |

## Database

**MariaDB is the supported database**, on Linux and on Windows: it is what
Ubuntu and Debian install, what CI tests (10.11 and 11.8) and what the
installer, procedures and collation fixes are written for. MySQL is not
supported. Its stricter defaults (`NO_ZERO_DATE`, `ONLY_FULL_GROUP_BY`), new
reserved words, deprecated `utf8mb3` and `caching_sha2_password` would mean
schema-wide changes with no gameplay benefit, and MySQL 8.0 is end of life.
Optional later: a CI job installing on MySQL 8.4 LTS to measure the gap,
best combined with the move to prepared statements in phase 3.

## Phase 1: reproducible build

Done for Linux: CMake with imported targets, presets, vcpkg manifest with
Lua 5.1 and tolua++ overlay ports, `build_server.sh`, verified heightmap
downloads, CI on Ubuntu 24.04 and 26.04 plus the vcpkg route.

**Windows (in progress, `docs/WINDOWS.md`):** `windows-debug` and
`windows-release` presets (MSVC with Ninja, vcpkg), `build_server.ps1`
(finds Visual Studio, builds, tests, downloads heightmaps with SHA-256
checks), `server.ps1` to start and stop the servers, data staged by the
build, a Windows CI job; the old Windows tooling and bundled executables are
removed. Confirmed with a client: servers built
and run on Windows against the Linux VM's database. Remaining: a local
MariaDB on the Windows PC (`setup.sh` runs under Git Bash, or a
`setup.ps1`), so development needs no second machine; `-Solution` builds
with a side-by-side older MSVC (docs/WINDOWS.md).

## Phase 2: runnable server for a community

Done: `mmoserverdb/setup.sh` (install, accounts, galaxy address, resource
spawning, reloading procedures), SQL repairs, `server.sh` (start, stop,
status, debug sessions in tmux with gdb), and the runtime fixes listed in
`docs/KNOWN_ISSUES.md`.

Remaining:
- **Remote database access** for the Windows development setup: done,
  `setup.sh allow-host <address>` / `deny-host`, or an SSH tunnel
  (`docs/WINDOWS.md`).
- **Security** (code review section 4, before any public server): done,
  in testing: servers binding only to `BindAddress` instead of every
  interface (`KNOWN_ISSUES.md` #3); unknown usernames no longer creating
  accounts by default (#3a); bounds-checked `Message` reads (#3b); player
  text in prepared statements instead of fixed SQL buffers (#3c); Argon2id
  password hashes, old ones replaced at the next login (#3e). Left: login
  attempts are not rate-limited, and each check costs about 64 MB and a
  few tens of milliseconds on the LoginServer's main thread.
- **Character limits**: the per-account limit is `account_characters_allowed`
  (default 2), changed by hand in SQL today. `setup.sh` options for the
  default limit of new accounts and for one account's limit, so test servers
  can be generous; later the same in the management portal.
- **Test-only terminals**: a configuration switch that removes the
  character builder terminals on live servers and keeps them on test
  centres.
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
- **Combat and creature AI review**: started under phase 5 (lairs spawn,
  creatures roam, aggro, fight back and leash; `KNOWN_ISSUES.md` 5f, 5j).
  Left: creature behaviour as `ai/` scripts, and a player staying in combat
  after the opponent dies (5i, to retest).
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

**Client data extraction tool.** Reads the client's `.tre` archives once
(IFF templates, datatables, string files, terrain) and fills or checks the
database tables, so servers don't need the client files at runtime and the
data can be traced to the client. Each row records which client files and
version it came from. This also lets communities add content from the
expansions, with one limit: the server can only use what the players'
installed client has, since the client draws everything itself. Jump to
Lightspeed (2003) is pre-CU and in the Publish 14 client (parked, see "Next
steps"); Rage of the Wookiees and Trials of Obi-Wan came after the CU and
need a later client and its protocol changes, so they are a per-server
choice, not a default. `tools/client/tre.py` already reads the archives.

## Phase 5: pre-CU gameplay

Combat first (the heart of pre-CU; under way, see the status above and
`docs/COMBAT.md`), then the order in "Next steps": the world's creatures,
factions and the GCW, the rest of the combat professions, pets,
communities, content, then Jedi. `docs/PRECU_AUDIT.md` has the state of
each system. Crafting and the entertainer systems show what a complete
system looks like in this codebase.

## Tools

### Management portal

For admins, QA and community leaders; a web application, reachable only
from trusted networks or behind a login, never open by default.

Started in `portal/` (`portal/README.md`): players' own accounts (password,
email, reset by email, optional registration) and, for staff, accounts
(create, ban, reset password, character limit, in-game CSR flag), roles
made of permissions, and an audit log; `portal/install.sh` checks a Linux
server and installs it. Still to do from the list below, and later ideas
from the community: an in-game item shop, in-game mail, a skill
calculator.

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
- **Terminal configuration**: a page to choose which terminals are placed
  and what they offer (for example the character builder terminals, with
  options added as features are tested).
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
