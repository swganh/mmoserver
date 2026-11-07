# Known issues

Problems found while running the servers with a real client (pre-CU SWGANH
client; servers on an Ubuntu 26.04 VM with MariaDB 11.8, and on Windows 11
built with Visual Studio 2022). Kept here
because Issues are disabled on this fork; move them to GitHub Issues when
they are enabled.

Status: **open**, **fix needs testing** (fixed in code, not yet confirmed
with a client), or **fixed**.

---

## Connection and stability

### 1. Client disconnects 20-30 seconds after entering the tutorial — not seen since #10 was fixed

After loading into the tutorial and standing idle, the client is
disconnected. The zone keeps running with the player link-dead.

- In one run the ConnectionServer dropped the tutorial zone's link after 60
  seconds without packets (`Session disconnect last received packet > 60`,
  then `Removing Session(127.0.0.1, 5010)`), taking the client with it.
- In a later run only the client session was removed and re-added; no
  timeout was logged.
- An idle zone with no players stays connected (tested for over 3 minutes),
  so the server-to-server keep-alive works without players.

Next step: capture `logs/connection.out` and `logs/tutorial.out` right after
the disconnect, and compare with the client's own log if it has one.

### 2. Zone crashes when a player reconnects — fixed

Confirmed with a client: disconnecting and reconnecting no longer crashes the
tutorial zone.

Reconnecting while the character is still link-dead crashed the zone in
`SpatialIndexManager::InitializeObject`
(`CharacterLoginHandler::_processSelectCharacter`). On disconnect,
`WorldManager::addDisconnectedPlayer` deletes the player's private tutorial
NPCs with `destroyObject`, which left them behind in two places:

- the spatial grid kept a pointer to the freed NPC (fixed in ed5a637:
  `destroyObject` removes every non-player object from the grid);
- the cell kept the NPC's id, and `BuildingObject::getAllCellChilds`
  returned a null pointer for it (fixed: `destroyObject` removes creatures
  from their cell, and `getAllCellChilds` skips ids that no longer resolve).

### 2a. Text sent by the client was garbled on Linux — fix needs testing

`BString` stores the client's UTF-16 text as 16-bit units but converted and
measured it with `wcstombs`/`wcslen`/`wcsncpy`, which use `wchar_t`: 16 bits
on Windows, 32 bits on Linux. Every client string (names, chat, command
arguments, the starting city) came out garbled on Linux. Effects seen:
`getattributesbatch` "bad lexical cast", and skipping the tutorial sending the
player to a garbage planet, which crashed the tutorial zone (waypoint foreign
key error, then a null database result) and the ConnectionServer
(out-of-bounds zone id). Fixed in `BString` (with unit tests that fail on the
old code), plus guards in all three crash sites. May also explain other
client input that "did nothing", such as the debris (#4).

### 2b. A character that failed to load could never log in again — fixed

After an interrupted transfer, the character was left on the zone's
"login in progress" list (`CharacterLoginHandler::playerZoneList`), so every
later login for it was silently ignored until the zone restarted. Entries
now expire after 30 seconds, with a warning logged when a login is retried.

### 2c. Switching character while the previous one is link-dead hangs — fixed

Disconnect (without logging out), then enter a zone with a different
character from the same account while the first is still link-dead there
(up to `Zone_Player_Logout`, 300 seconds): the loading screen never ends.
The zone saves and removes the old character, then is meant to load the new
one, but the check for that step in
`WorldManager::storeCharacterAttributes_` was inverted (`!clContainer`), so
the new character was never requested. The callback also captured the
container by reference to a parameter that no longer existed when it ran.
#2b's entry for the new character was left behind as a side effect.
Confirmed with a client: disconnect, create a new character, transfer to
Tatooine.

Untested edge case: if the old character is still *connected* (not
link-dead) when the account selects another, both share one client object,
and removing the old character deletes it.

### 2d. Servers crashed after a long idle period — fix needs testing

Left running for a day with no players, the ConnectionServer died with
`Lost connection to MySQL server during query` and a segfault. MariaDB
closes connections idle longer than `wait_timeout` (8 hours by default),
and the servers never checked theirs, so the next query failed. Any failed
asynchronous query then crashed the server: `Database::destroyResult` read
the null result. Connections idle for over 30 seconds are now checked
before use and replaced if closed, and failed queries no longer crash.
Reproduced and verified with `wait_timeout` set to 40 seconds.

### 3a. Unknown usernames create accounts — open (security)

`sp_ReturnUserAccount` creates an account for any username it doesn't know,
with the password typed. Convenient on a test server; a public server needs
it off. Make it a setting (default off for public installs).

### 3. Servers listen on all interfaces — open (security)

`Service.cpp` binds every UDP socket to `INADDR_ANY`, whatever
`BindAddress` says, so the internal cluster ports (ConnectionServer 5000,
zones 50xx) are reachable from the network. Only 44990-44992 should be
public. Until fixed, keep those internal ports closed in the firewall.

## Tutorial and gameplay

### 4. Debris blocking the tutorial path has no Attack option — fixed

Not a bug of its own: the script makes the debris attackable only after you
talk to the Covard officer (`makeCreatureAttackable` at sub-state 17), and
the officer was invisible (#5).

### 5. Tutorial NPCs did not appear — fixed

The tutorial's 17 characters (officers, the nervous guy, the trainer's
officer, ...) are persistent NPCs in the tutorial building. NPCs loaded with
a building's cells were added to the cell but never to the spatial grid,
and only the grid sends creatures to players; cell registration only sends
tangible items. So no player ever saw an NPC inside a building that came
from the database, on any planet. The debris and bandit were visible
because the script spawns them later through the normal path. Fixed:
once a building is in the world, its cells' creatures are added to the grid.
Confirmed with a client for the first rooms; see 5b-2.

Talking to any NPC inside a building failed with "You are out of range":
the check compared the NPC's building with the player's cell. Fixed.

### 5b-2. Some NPCs inside buildings were invisible — fixed

In the tutorial's item room the officer (47513079097) was invisible while
the supply drums next to him showed, yet the client knew him (his barks
appeared in chat). The grid returns creatures before buildings, so on login
an NPC in a cell could be sent before the client had the building and that
cell, leaving the NPC outside the building. Creates around a player now
send buildings and other objects first. Confirmed with a client: a new
tutorial character sees the item room officer and the other NPCs; a trace
showed all 63 NPCs of the building sent at login and nothing destroyed.

Also seen: `zmap::GetCellContents` tests the type mask with `&&` instead of
`&`, so it returns every type whatever is asked; one stormtrooper's bark is
just "[]:". The first hall's 36 stormtroopers ("Joe...") are not in the
original tutorial, but the script does use some NPCs from the same block
(the item room and bank room officers).

### 5c. Object positions inside buildings are mirrored — open

`Object::getWorldPosition` computes a building's contents at (-x, -z) of
where they should be (for an unrotated building, world = (-x, y, -z)
instead of (x, y, z)). Everything inside uses the same formula, so players
and NPCs in a building see each other, but distances between indoor and
outdoor objects are off by up to twice the indoor offset.

### 5d. Sampling often yields nothing at low concentration — open

Works, but needs a high concentration to return resources reliably; the
formula in `ArtisanManager::sampleEvent` may need tuning.

### 5a. No harvestable resources on a fresh install — fix needs testing

The Windows installer spawns resources only with the optional
`setup.bat /generateresources` flag, and `setup.sh` didn't do it at all, so
the `resources` table was empty. `setup.sh install` now calls
`sp_ResourceInitialSpawn`; existing databases can run
`setup.sh spawn-resources` (then restart the zones).

The spawn procedures also computed each resource's end date as
`RAND() * (time_min - time_max) + time_min` (min and max swapped), which put
about 60% of new resources' `spawn_end` in the past. Fixed in both
procedures.

### 5b. Resources never rotated — fixed

Zones loaded the active resources at startup and kept them until restart;
nothing checked `spawn_end`. Now `sp_ResourceRotate` retires each expired
resource and spawns a replacement of the same type on the same planets. Each
zone calls it at startup and every 10 minutes
(`Zone_Resource_Rotation_Interval` in `config_server`, in seconds), then
loads the new resources and retires the expired ones, which drop out of
survey lists. Harvesters on an expired resource stop (`sf_HarvestResource`
already did this), and a sample in progress ends. Tested on Naboo with
expired resources. Confirmed on the VM: a resource expired by hand was
replaced within one interval while the servers ran (`Replaced 1 expired
resources`, and Tatooine loaded the new Lok resource).

Existing databases need the new procedure: `setup.sh update-procedures`.

Not implemented: depletion. Sampling lowers `unitsLeft`, but a resource is
never retired when it runs out.

### 5e. Tutorial prompts lag behind the player — open

The script plays each voice-over and waits for it, and some prompts wait
for the player to reach a spot, but the debris is made attackable at the
start of its step (the later call in `tutorial/part1.lua` is commented
out). A quick player is ahead of the prompts. Pacing, not an engine delay.

### 5f. Combat creatures never appeared (tutorial bandit) — fix needs testing

Combat creatures (`AttackableCreature`: the tutorial bandit, creatures from
lairs) are created unspawned and queued as dormant; the AI manager calls
their `handleEvents`, which was empty, and nothing else called `spawn()`.
So none ever appeared. They now spawn on the first handling.

The rest of their AI is mostly missing too: nothing moves them out of the
idle state, so expect creatures that can be attacked and killed but barely
react. Tatooine's 4 lairs will now put creatures in the world for the
first time.

Confirmed with a client: the tutorial bandit appears, fights, and the
script continues after his grenade.

### 5i. Still in combat after the opponent dies — open (workaround: `/peace`)

After killing the tutorial bandit the player stays in combat, which blocks
leaving the tutorial. When a creature dies, `die()` makes peace with its
defenders (clears the defender lists, sets the Peace state) but, unlike the
`/peace` command (`ObjectController::_handlePeace`), does not switch off the
attacker's auto-attack, clear the combat target or resend the player's
state. `/peace` or the Peace button gets the player out of combat. For the
combat and creature AI review.

### 5j. Lairs never spawn — open

Tatooine has lairs in the database (two with a count of 25, spawning
anywhere in the region from about -2200,-4700 to -1500,-3950), and they are
loaded and queued, but none ever appears: `LairObject::handleState` only
counts down the initial spawn delay in `State_LairUnspawned` and nothing
then calls `spawn()` or moves the lair on; `handleEvents` only handles
death. The same kind of missing state machine as `AttackableCreature`
(5f). For the combat and creature AI review; spawning up to 50 lairs and
their creature waves needs the creature AI to work.

### 5k. Travel tickets to points with an apostrophe were never created — fixed

Buying a ticket from Tatooine to Lok's "Nym's Stronghold" took the credits
but no ticket appeared: `ObjectFactory::requestNewTravelTicket` put the
travel point names into the SQL unescaped, and the apostrophe broke the
query (Tatooine logged the syntax error). Point names are escaped now.

Still open: the ticket is paid for before it is created, so any failure
keeps the credits. Charge on success instead.

### 5l. Travel to another planet by ticket never happened — fixed

At the ticket collector the destination window opened, but OK did nothing.
The ConnectionServer approved the transfer, then the zone saved the
character and stopped: `WorldManager::savePlayer` (rewritten in an earlier
refactor) no longer called back for a zone transfer, so the player was never
handed to the destination zone, and it stored `planet_id=0` instead of the
destination planet. The transfer now stores the position (destination
planet, outside any building), then the attributes, then hands over. The
ticket window's failure paths (surveying or sampling, no matching ticket,
unknown travel point) now log the reason and tell the player instead of
doing nothing. The tutorial's travel uses a different, synchronous path and
was not affected.

### 5h. "Too many commands queued. A command was dropped." — open

Seen repeatedly while fighting the tutorial bandit. The player's command
queue overflows during auto-attack; check its limit and what fills it.

### 5g. No explosion when the tutorial debris is destroyed — fix needs testing

The script now plays `clienteffect/combat_explosion_lair_large.cef` on the
debris when it is destroyed (`docs/TUTORIAL.md`).

## Login

### 6. "Not allowed to create any more characters" — fix needs testing

The client shows this when the ConnectionServer (which character creation
connects to) refuses the account or reports no free slot. Causes found and
fixed:

- The LoginServer cleared `account_authenticated` after every character
  delete, so the account counted as logged out until the next login.
- The ConnectionServer refused any account still marked logged in
  (`account_loggedin`), which it clears only when it notices the previous
  client has gone (up to the 60 s session timeout). Seen on the VM:
  refused after logging out and in and after restarting the client, fine
  a little later. A reconnecting account now takes over its stale session,
  and every refusal is logged with its reason in `connection.out`.
- Characters were counted differently from the character list (which
  needs a `character_appearance` row), so half-created characters could
  use slots invisibly; an account with no characters got a NULL limit.
  The count now matches the list.
- The login check ran two async queries and handled them in arrival
  order; they could be swapped. One query now.
- The LoginServer had the same stale-flag problem one step earlier: it only
  accepted a login while the account was marked logged out
  (`account_authenticated = 0` and `account_loggedin = 0`), so after killing
  the client the client said "The Login Server is currently not available"
  until the old session timed out (seen on the VM: three failed logins,
  then success). A correct password now logs in; the ConnectionServer
  replaces the stale session. An old LoginServer session timing out no
  longer clears the authentication of a newer login of the same account.

## Protocol

### 7. Unhandled client messages — open

| Server | Opcode | Notes |
|---|---|---|
| ZoneServer | 0x6B9E5323 (1805538083) | at zone-in and on reconnect; not defined anywhere in the source |
| ZoneServer | 0x0F5D5325 (257774373) | at zone-in; not defined anywhere in the source |
| ChatServer | 0x48F493C5 (1223988165) | `CmdSceneReady`, routed to chat as well; harmless |

The logs print opcodes in decimal; printing them in hex would make them
easier to match against protocol documentation.

## Data

### 8. Startup data warnings — open

- Tatooine: `Unable to load spawn regions with id: 8` (another path then
  loads `8 Creature Spawn Regions`) and `Unknown group [5]` (an object with a
  category the zone doesn't recognise is skipped).
- `SpawnRegionFactory::requestObject` reads `spawn_regions.mission` with
  `getUInt(9)`, but column 9 of its SELECT is `parent_id` (`mission` is
  column 10). Looks like an off-by-one.

## Memory

### 10. A tutorial ZoneServer grew to 6.2 GB — fix needs testing

Looting all from a corpse (the tutorial bandit) put the zone in an endless
loop: `ObjectController::lootAll` advanced its iterator only for objects
that were not items, and nothing removed the item from the corpse's
inventory, so it requested a new copy of the first item for the player
again and again. Every request queued a database job: the zone grew by
about 400 MB a minute, stopped answering the player (still in combat, no
chat, loot window gone, back to character selection), the VM swapped,
links timed out after 60 s (issue #1 may be the same) and the kernel
killed the zone. Caught with heaptrack on the VM: 1.74 GB from
`Database::executeAsyncSql` via `requestNewDefaultItem` from `lootAll`.

Fixed: loot all loops over a copy of the corpse's items, takes each one out
of the corpse's inventory and frees it, and stops when the player's
inventory is full. Taking a single item by dragging it now frees the
temporary item too.

Keep the memory log running during testing to catch anything else:

```bash
nohup bash -c 'while true; do echo "$(date +%T) $(ps -o rss=,args= -C ZoneServer | tr "\n" " ")"; sleep 60; done' >> ~/zone-mem.log 2>&1 &
```

## Cosmetic

### 9. `MessageFactory Heap at <garbage>` log lines — open

Every server prints this with a meaningless number (e.g. `7.27059e+31`);
the value being printed is not initialised or not a heap figure.

---

## Windows

### 11. Zones crashed on Windows as soon as an NPC moved at negative coordinates — fixed

`zmap::_getCellId` converted a negative float position straight to an
unsigned integer, which C++ leaves undefined. GCC happened to wrap it, and
adding half the map size wrapped it back to the right grid cell, so Linux
worked; MSVC did not, the index left the lookup table and the Tatooine zone
died with an access violation right after startup. It now converts through a
signed integer and clamps to the grid (the same cells as before for every
position on the map). `zmap::addRegion` stored region positions in unsigned
integers too, which put regions starting at negative coordinates in the
wrong cells on Linux as well.

### 12. Bugs found by MSVC warnings — open

The first Windows build warns about code that is wrong on every platform
(GCC does not warn at the project's warning level):

- `CloningTerminal.cpp:183` passes the `Database` object where the SQL
  expects the schema name (`%s`), so choosing a cloning facility runs broken
  SQL.
- `TradeManagerChat.cpp:1383` passes the galaxy name twice, so the auction id
  in the query is the wrong argument (bazaar auction lookup).
- `StateManager.cpp:47-49`, `GroupObject.cpp:263` call `empty()` (a test)
  where `clear()` was meant.
- `AttackableCreature.cpp:999`, `MissionManager.cpp:1189-1446` use
  uninitialised variables (`destination`, `mission_start`, `mission_dest`).
- `OCAdminHandlers.cpp:85`, `OCCharacterSheethandlers.cpp:293`,
  `OCStructureHandlers.cpp:300`: scanf formats that do not match their
  arguments (narrow strings read with wide formats).
- `HashString.cc:45`, `anh/hash_string.cc:26` store a 32-bit hash in a
  pointer.

To fix with the warning cleanup in phase 3, or sooner for cloning and the
bazaar when those are tested.

### 13. `build_server.ps1 -Solution` with a side-by-side older MSVC — open

A generated Visual Studio solution builds with Visual Studio's default MSVC,
which can be an older one installed side by side, while vcpkg's libraries
use the newest; linking then fails. The Ninja build is not affected.
Workaround in `docs/WINDOWS.md`.

## Zones and scripts

### 14. Tatooine crashed once while a character loaded in — fix needs testing

Seen once on the Linux VM; the log ended without a reason and it did not
happen again under `./server.sh debug -g`. One candidate, found while adding
script events: when an object left the world, `zmap::RemoveObject` erased a
region that no longer existed from the object's region list and then called
`onObjectLeave` on it anyway, through a null pointer. It now skips that
region. If a zone crashes again, run it under `debug -g` and keep the log.

### 15. Scripts' `print` output never reached the logs — fixed

Lua's `print` wrote to stdout, which the servers buffer into the log file;
the buffer was never written before the server stopped, so script messages
(`WeatherController v.0.2` and the like) were lost. `print` now goes through
the server's logger and appears in the zone log as `[script] ...`.

### 16. Server-side regions never matched (cities, badges) — fixed

Found when `region_entered` did not fire in Bestine. The region map
(`zmap`) treated every region as a rectangle with its corner at
`planet_regions.x/z`, with width and height swapped, so nobody was ever
inside a city and badge squares were shifted by half their size. It also
stepped grid rows by 410 cells where a row has 411, and `RemoveRegion`
(camps) never removed anything.

Fixed: regions follow the convention the no-build check already used:
x/z is the centre, `width` 0 makes a circle with `height` as its diameter,
otherwise a width x height rectangle around the centre
(`IsPointInRegion`, `GetRegionBounds` in `Utils/MathFunctions`, unit
tested).

City areas: the cities' rows were the larger no-build areas (Bestine 750 m
radius), not the areas the client names. Migration 003 gives Tatooine's
cities the client's own circles from `datatables/clientregion/tatooine.iff`
(Bestine 375 m), generated with `tools/client/city_regions_sql.py`; the
no-build rows are unchanged. Confirmed on the VM: Bestine's entry and exit
match the client's messages, and the Lars homestead badge is awarded.
Still to do: the other planets (their client files are needed), and the second circle the client draws for Mos Eisley
and Mos Entha (the server has one area per city).

### 17. Corpse loot can't be moved from the opened container — open

Seen on the tutorial bandit. Loot all works; opening the corpse's
container shows the items, but they can't be moved to the inventory. The
single-item path changed with #10 (dragging an item frees the temporary
copy); check that path first, with `tutorial.out` from a drag attempt.

The events confirm it is the server: picking up items from the opened
bandit corpse (`object_used menu_id=10`, three tries on two items) raised no
`item_transferred`, so the transfer never completed.

### 18. Tutorial combat is slow (debris, bandit) — open

Destroying the debris and killing the bandit take much longer than in the
original. Probably their HAM against a new character's damage with the
limited combat we have; check the debris and bandit stats in the data
against the client and recordings before changing them.

### 19. The bandit survives his own grenade — needs testing

The pirate's lines (`newbie_convo` `pirate_taunt3`, "A GRENADE!", then
`pirate_taunt4`, "This grenade is defective! Oh no!") set up his death by
his own grenade; here he keeps fighting. The script throws it when either
side runs low (200 HAM) or after five minutes of fighting: the lines, the
explosion, then `die()`, which is also how he dies in normal combat. Next
test: does the grenade go off, and does he fall? `tutorial.out` has
`creature_died` for him when he does.

### 20. A line of stormtroopers in the tutorial's first room — to check

The database places 36 stormtroopers in the first cell of the tutorial
station (`persistent_npcs`, parentId 2203318222960). The original
project's data put them there, not the script. Check against recordings
of the live tutorial and remove or thin them with a migration.

### 21. Racing ahead in the tutorial got it stuck — fixed in parts 1-2, parts 3-9 need testing

The script armed each check (container open, chat, food, client reports)
only when it reached that step, so anything the player did earlier was
missed and the script waited for it forever: the drum in room 2, after
which the officer handing over the weapon would not talk. Parts 1 and 2 now
run on events and check what the player has already done
(`docs/TUTORIAL.md`); confirmed on the VM, racing through rooms 1-2 to the
end of the tutorial. Parts 3-9 now run on events too, setting the
sub-states C++ checks (the weapon, the release documents, the
Quartermaster) as soon as the player arrives.

## Fixed while getting the servers running

For reference; details are in the commit messages on `phase2-db-installer`.

- Collation errors on MariaDB 11.5+ (`Illegal mix of collations`).
- Server config and Lua script file names that only matched on Windows
  (case-insensitive file systems).
- Stack overflow loading Naboo's spawn regions.
- Servers started in the background were stopped by the shell (SIGTTOU).
- ChatServer start-up race that left the galaxy "Loading...".
- A malformed client command (`getattributesbatch`, emotes, mood) crashed
  the zone; commands now fail individually.
- LoginServer crashed after a character was deleted (missing `break`).
- Characters with a surname could not be deleted: the character list sent
  "First " (the surname was commented out, the space was not), so the
  client's "delete <name>" check could never match what was typed.
- NPC and creature posture changes never reached clients (only players'
  did), so killed creatures stayed standing.
- Killed creatures never disappeared from clients: since the 2015
  "Backports" commit, `WorldManager::destroyObject` no longer removed
  creatures from the world, so no destroy was sent. The tutorial debris
  died on the server but stayed on screen.
- Targets brought to exactly 0 health were neither incapacitated nor
  killed (only going below 0 counted), so the tutorial debris stayed at 0
  and the script waited forever.
- Tutorial zone crashed in the item room when the script showed "open me"
  over the drum: fly text cast its source to a creature (null for a
  container). Fly text in instances also only reached grouped players.
- Missing `sp_DeleteMail`; SQL procedures broken by earlier bulk edits.
