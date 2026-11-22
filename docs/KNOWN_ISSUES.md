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

### 2e. Zone crashed with "free(): invalid pointer" after a player search — fixed

The player search (the client's character match request) added each
matching player in range through a pointer it never set, so with a second
player nearby it wrote into stray memory; the zone aborted soon after
(`free(): invalid pointer` in `_handleRequestCharacterMatch`, found with
`server.sh debug -g` on the test VM). It now adds them to its own list.
The two strings it reads from the request are bounded to their buffers,
and a player whose title is not plain text no longer crashes the match.
Confirmed on the test VM: a search with two characters nearby works.

### 2d. Servers crashed after a long idle period — fix needs testing

Left running for a day with no players, the ConnectionServer died with
`Lost connection to MySQL server during query` and a segfault. MariaDB
closes connections idle longer than `wait_timeout` (8 hours by default),
and the servers never checked theirs, so the next query failed. Any failed
asynchronous query then crashed the server: `Database::destroyResult` read
the null result. Connections idle for over 30 seconds are now checked
before use and replaced if closed, and failed queries no longer crash.
Reproduced and verified with `wait_timeout` set to 40 seconds.

### 2f. Friends shown offline; no message when a friend comes online — fixed

A friend who was already logged in showed as offline in the Community
window's Friends List: `ChatManager::checkOnlineFriends`, which tells a
player at login which friends are online, was commented out. It runs again
at scene ready. The "friend has come online" message did not appear either,
even with "System message of online status" ticked (the client shows it
when the status update matches a name on the list). The update sent the
friend's name as written ("Brawler") while the list holds it in lower case
("brawler"): it now sends the lower-case name. At login the friends
online are sent (only those: the client announces every status it gets,
so sending the offline ones too printed "... has gone offline" for each
at every login). Adding or removing a friend sends the list before the
statuses (the list, sent last, reset them to offline). Left: a client that
relogs without restarting keeps the statuses it had, so a friend who logged
out meanwhile shows online until they next log in or out. The
notification box is kept by the client (it survives a restart).
Confirmed on the test VM: a friend logging in shows online with
"... is online at ...", and one already online shows online at login.

### 2h. ChatServer read past the end of the first connect message — fixed

The connect the ConnectionServer sends before a character is chosen holds
only a character id of 0; the ChatServer read a planet id after it before
checking, which the bounds checks (#3b) logged as
`Message read past its end: 4 bytes at 12 of 12` at every login. It reads
the planet id only for a real character.

### 2g. Chat rooms created with no name, listed again on every refresh — fixed

The ChatServer cut a new room's name out of the client's path
(`SWG.<galaxy>.<name>`) at 5 + the length of its own galaxy name. With the
client using a different galaxy name, the name came out empty, and the
Channel Browser added the nameless room again on every refresh. The name is
now taken after the galaxy segment whatever it is called; an empty name is
refused and logged; each create logs `Create chat room: path '...'`.
Migration 015 removes rooms already created without a name.

The likely cause: the ChatServer loaded the galaxy name and the channels
with two queries at once, and when the channels came first their paths had
an empty galaxy (`SWG..corellia`): the planet channels then showed outside
the galaxy folder, and a room created under one got a path the old code
could not cut. The channels now load after the galaxy name. Confirmed on the test VM:
the planet channels sit inside the galaxy, and a new room keeps its name.

### 2i. Bazaar handlers worked from a stale copy of the logged-in players — fixed

The ChatServer's bazaar and structure handlers copied the list of logged-in
players once, at startup (empty), and refreshed it only in some handlers.
Any other bazaar reply for a player who logged in after the last refresh was
dropped (`Error getting player from account map`): item details never
loaded ("Waiting for details download"), and a new listing's category, set
after it is created, stayed 0 ("Unknown Object Type"). They use the live
list now; migration 016 fills in the category, region and planet of
listings already made (seen on the test VM: a house deed listed with
category, region and planet 0). The details, retrieve and cancel handlers also returned when they
did find the player (#3d); those checks are the right way round.

Confirmed on the VM (Nov 2026): listing, details, bidding, buying,
retrieving and cancelling all work, with `'` and `"` in descriptions.

With the handlers working, retrieve and cancel took any listing id a client
sent: another player could take or cancel someone else's listing (found by
review of obi-two/mmoserver#23). Both are now for the listing's owner only
(the seller, or the winner of an ended auction), and a cancel mails only
that auction's bidders, not every bid its bidder ever made.

### 2j. Listing an item on the bazaar deleted it — fixed

After a listing was created, the ZoneServer moved the item to the bazaar and
then removed it from the zone with `ContainerManager::deleteObject`, which
also deletes its database row: the listing named an item that no longer
existed, so it could never be shown in detail, sold or retrieved (seen on the
test VM: a house deed listed, then no row in `items`). The item is now
removed from the zone only. Migration 017 removes the listings already left
without their item, with their bids; those items cannot be recovered.

### 2k. Zone crashed when a bazaar purchase reached the buyer — fixed

`ObjectFactory::requestTanoNewParent`, which moves a bought item into the
buyer's inventory (and a manufacturing schematic from a factory into the
datapad), read a row from the result of an `UPDATE`, which has none: a null
result set, and the zone crashed (found on the test VM with
`server.sh debug -g`; the item was already in the buyer's inventory in the
database). It now loads the object by its id once the parent is set. The
handler that calls it looked up the buyer's inventory before checking the
buyer was still there. Confirmed on the test VM: a melon listed by one
character and bought by another reached the buyer's inventory, and a house
deed bought before the fix was in the buyer's inventory at the next login.

### 3a. Unknown usernames create accounts — fixed

`sp_ReturnUserAccount` created an account for any username it didn't know,
with the password typed. Convenient on a test server; on a public server
anyone could take any name. Now it only does so when the LoginServer's
`AutoCreateAccounts` setting is on (`config/loginserver.cfg`, off by
default); otherwise an unknown username is refused like a wrong password,
and accounts are made with `sudo ./mmoserverdb/setup.sh add-account NAME`.
The LoginServer logs which at startup. Existing accounts are unaffected.
Confirmed on the test VM: refused when off, created with `AutoCreateAccounts=1`.
After pulling, `sudo ./mmoserverdb/setup.sh update` reloads the procedure
(its new third argument): without it, every login fails.

### 3. Servers listen on all interfaces — fixed

`Service.cpp` bound every UDP socket to `INADDR_ANY`, whatever
`BindAddress` said, so the internal cluster ports (ConnectionServer 5000,
zones 50xx, chat 5200) were reachable from the network. Each socket now
binds to its configured address:

- the services game clients use (LoginServer 44990, ConnectionServer
  44991, PingServer 44992) to `PublicBindAddress`, `0.0.0.0` (every
  interface) by default, as before;
- the ConnectionServer's cluster port to `ClusterBindAddress`
  (`127.0.0.1`), zones and chat to `BindAddress` (`127.0.0.1`).

Each service logs `Service listening on <address>:<port>`. Checked locally
with `ss -ulnp`: 44990-44992 on 0.0.0.0, 5000 on 127.0.0.1. Servers split
across machines set `BindAddress`/`ClusterBindAddress` to a LAN address
(and the `config_process_list` addresses to match). A firewall on the
internal ports is still good practice. Confirmed on the test VM.

### 3b. Malformed packets could read past the end of a message — fixed

`Message` reads (`getUint32`, `getStringAnsi`, ...) trusted the packet:
a number read near the end, or a string whose length said more than the
message carried, read whatever memory followed. A Unicode string length
near 65535 also overflowed `BString`'s 16-bit allocation size, writing
past the end of a heap buffer. Every read is now checked against the
message size: a read past the end returns 0 or an empty string, the
message is marked (`isOverrun()`), and the server logs
`Message read past its end: ...` once per message. `Packet` reads are
checked against the packet buffer the same way. Unit tests in
`src/NetworkManager/message_unittest.cc`. Watch the logs for that warning
in normal play: a well-formed client should never cause it.

### 3c. Player text was built into SQL — fixed, needs testing

Names, mail, biographies, chat channels, bazaar listings and the like
were escaped into fixed-size stack buffers (a long enough name or mail
overflowed them), and some were not escaped at all: the launcher login
put the username and password into the SQL as sent (any account could
be logged into), and the planet map, channel path, character-creation
models, permission lists and image-design hair were unescaped. Player
text is now bound as values of prepared statements
(`Database::executePreparedAsync`/`executePrepared`): it cannot change a
statement and has no length limit in the server. A database refusal (a
name too long for its column, say) fails that one request and logs
`Prepared statement failed`, instead of stopping the server.
Transactions bind their queries too (a `%` in a bazaar description was
read as a format directive). Checked against MariaDB: injection text in
either login gets no account, a 70,000-character username is refused
cleanly, binary mail attachments and transaction text round-trip
exactly. Text the server itself holds (zone names, resource names,
serial numbers) still goes into SQL as before.

Along the way: player mail stored the sender's name twice, the bazaar's
Offers window was a syntax error for every name, a debug line logged the
launcher password, and every finished database job leaked its query
string.

## Tutorial and gameplay

### 3d. Two SQL statements had the wrong arguments — fix needs testing

Found by the compiler's format checks:

- Storing clone data at a cloning terminal passed the `Database` object
  where the schema name belongs, so the `CALL` was invalid SQL, and an SQL
  error stops the server. It now names the schema.
- Retrieving a bazaar item passed the galaxy name twice, so the auction id
  was never used. Fixed; but that handler (and the bazaar's cancel and
  item-info handlers) return when the player *is* found, so they never
  reach the query for a logged-in player. Left as it is until the bazaar
  is looked at as a whole.

### 3e. Passwords were stored as unsalted SHA-1 — fixed

`account_password` held `SHA1(password)`, compared in the database: no
salt, and fast to guess, so a leaked account table gave away most
passwords. Passwords are now Argon2id hashes (libsodium, its interactive
limits: about 64 MB and a few tens of milliseconds per check), checked
by the LoginServer. The new `sp_AccountForLogin` returns the account
whatever the password, with whether it matches an old SHA-1 one; that is
accepted once and replaced by an Argon2id hash at that login (the log
says `password now stored as an Argon2id hash`). A login to an unknown
username spends the same time, so the reply time does not tell which
usernames exist. `setup.sh add-account` stores Argon2id (from
`LoginServer --hash-password`), or SHA-1 if no LoginServer is built yet.
Migration 014 widens the column. Needs `libsodium-dev`. Confirmed on the
test VM.

Not done: login attempts are not rate-limited, and the check runs on the
LoginServer's main thread. `sp_ReturnUserAccount` is no longer used (an
older LoginServer still works with it until its account is re-hashed).

While there: the launcher's session-key login read the session key as the
account's CSR flag, and a `DFT_string` data binding copies the whole
column whatever the field's size (the password field was sized for the
hash; the other bindings are unchanged).

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

### 5i. Still in combat after the opponent dies — fix needs testing

After killing the tutorial bandit the player stays in combat, which blocks
leaving the tutorial. When a creature dies, `die()` makes peace with its
defenders (clears the defender lists, sets the Peace state) but, unlike the
`/peace` command (`ObjectController::_handlePeace`), does not switch off the
attacker's auto-attack, clear the combat target or resend the player's
state. `/peace` or the Peace button gets the player out of combat.

Seen again in the tutorial after the debris and the bandit (combat music
until it timed out). Fixed: when a creature dies, each player who fought
it and has nobody else to fight leaves combat as with `/peace` (combat
target cleared, auto-attack off, Peace state, state resent).

Not enough: on the VM the combat music still started as the bandit died
(to his own grenade) and the player had to press Peace. The music starting
*at* the death suggests something puts the player back into combat after
`die()` (auto-attack firing again, the bandit's AI, or the grenade path,
where `npcStopFight` does nothing), or the player was not in the bandit's
defender list.

Found: a creature's AI never left combat, not even dead. `State_Dead`
was never set, so the corpse's AI kept attacking every tick, and
`NpcManager::_verifyCombatState` checked whether the target was dead but
not the attacker: each attack put the player back into combat and back on
the corpse's defender list, undoing the peace, until the corpse was
removed minutes later. Now a dead or incapacitated creature's AI moves to
`State_Dead` and stops, and `_verifyCombatState` refuses attacks from a
dead or incapacitated NPC.

### 5j. Lairs never spawn — fix needs testing

Tatooine has lairs in the database (two with a count of 25, spawning
anywhere in the region from about -2200,-4700 to -1500,-3950), and they are
loaded and queued, but none ever appeared: `LairObject::handleState` only
counted down the initial spawn delay in `State_LairUnspawned`, and nothing
then called `spawn()` or moved the lair on. Now, when the countdown runs
out (3 to 6 minutes after the zone starts), the lair and its first wave of
creatures go in the world (`Lair ... spawned` in the log). Locally 34 lairs
had spawned after 7 minutes, without errors.

The creature AI was the other half (5f): nothing ever moved a creature out
of idle, so creatures never fought back, roamed or attacked. Now
(`AttackableCreature::_think`, each tick):

- attacked, a creature fights back: it runs at its target (its
  `creature_stalking_speed`) and attacks in its weapon range;
- an aggressive creature (`creature_is_aggressive`) attacks a player who
  comes within `creature_attack_range`;
- it gives up when its target dies, is incapacitated or is further from
  its home than `creature_stalking_max_distance` (64 m without one) plus
  its reach, then walks home and leaves the players alone on the way;
- with players within 64 m it wakes up and roams (`creature_is_roaming`),
  and dozes off again when they leave.

The tutorial's bandit is left to its script. Log lines: `Creature ...
fights ...`, `Creature ... gives up and goes home`.

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

### 5h. "Too many commands queued. A command was dropped." — expected

Seen repeatedly while fighting the tutorial bandit and a rill on Tatooine.
The command queue (30 entries) was strictly first in, first out: a combat
command waiting for its cooldown at the front held up every command
behind it, including the non-combat ones the client sends all the time in
a fight, which should run at once. In a long fight they piled up past 30
and were dropped. Now, while the front combat command cools down, the
first non-combat command behind it runs; combat commands keep their order
and cooldowns.

Still seen on the VM with that change, with several Attacks in the
client's queue: non-combat commands were already put ahead of combat ones
when queued, so it is the attacks themselves. The zone now logs what fills
the queue when it overflows (`Command queue full for ...`: how many are
combat commands, the front one, the time to the next combat command, and
whether the queue is scheduled). `docs/COMBAT.md` has the combat review.

Not seen since the combat rework (Nov 2026). When it was seen, the attack
queue was being spammed on purpose: past 30 queued commands the client is
told one was dropped, which is the queue's limit working, not a bug. The
log line stays for diagnosis.

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

### 17. Corpse loot can't be moved from the opened container — fixed

Seen on the tutorial bandit. Loot all works; opening the corpse's
container shows the items, but they can't be moved to the inventory. The
single-item path changed with #10 (dragging an item frees the temporary
copy); check that path first, with `tutorial.out` from a drag attempt.

The events confirm it is the server: picking up items from the opened
bandit corpse (`object_used menu_id=10`, three tries on two items) raised no
`item_transferred`, so the transfer never completed.

Cause: the transfer first asks whether the player may take things out of
the item's container (`ObjectController::checkContainingContainer`). A
creature's inventory is not in the world's object map, and that branch
only accepted the player's own inventory, so every pick-up from a corpse
was refused (loot all takes another path). It now also accepts a dead
creature's inventory when the player has loot rights, the same check as
the corpse's loot radial.

Loot all then put the items in the inventory but left them showing on the
corpse: the corpse's loot window sends its items to the player without
registering the player as watching the corpse's inventory, and taking an
item only told registered watchers. The looter is now told directly,
for loot all and for single items. Others who had the corpse open (a
group) still see the old contents until they reopen it. Confirmed on the VM:
single items drag out of the bandit's corpse, and loot all empties it.

Creature corpses (womp rats and the like) hold credits only: the server
never puts items in them, and neither guide lists item loot for creatures
(the 2003 tables have no loot column, the 2004 ones meat, hide and bone,
which come from harvesting). `loot_groups` and `loot_templates` exist in
the database but nothing reads them, and their rows are placeholders
(group 1, used by the womp rats, holds Tusken boots). Credits are
10-34 at random for every creature (`NonPersistentNpcFactory`), not from
any source. Creatures did drop small amounts of credits pre-CU (as
remembered by a player of the time), so the credits stay; the amounts are
still to be sourced.

### 18. Combat is slow (debris, bandit, Tatooine creatures) — partly fixed

Destroying the debris and killing the bandit take much longer than in the
original. Probably their HAM against a new character's damage with the
limited combat we have; check the debris and bandit stats in the data
against the client and recordings before changing them.

Tatooine: the database had copied the womp rat's stats (1000 HAM, 90-110
damage) onto its smaller relatives, so a novice fighting a lesser desert
womp rat (Prima 2004 p. 180: level 5, 140-170 HAM, 45-50 damage) faced a
creature six times too tough; the rill had 500 health (p. 182: 180-220).
Migration 006 sets the womp rat (p. 184: level 10, 810-990 HAM, 90-110
damage, 356 XP), the lesser desert womp rat and the rill from the guide.
The womp rat itself was nearly right: it is a level 10 creature, a hard
fight for a novice. Still to do: the other creatures as their pages come
(greater desert and variegated womp rats are not in the pages at hand),
and the tutorial's debris and bandit.

The combat rules (`data/script/combat/rules.lua`, `docs/COMBAT.md`) replace
the fixed 67% hit, the random pool and the 2 s attack: hit chance from the
weapon's range modifiers, skill mods and postures; damage on one pool,
mostly health; the weapon's own attack speed. Migration 007 gives the
weapons their ranges in metres and new characters the noob CDEF (50-80,
p. 392). Next test: a novice with the noob CDEF against a lesser desert womp
rat and a womp rat, and the bandit.

### 19. The bandit survives his own grenade — fixed

The pirate's lines (`newbie_convo` `pirate_taunt3`, "A GRENADE!", then
`pirate_taunt4`, "This grenade is defective! Oh no!") set up his death by
his own grenade; here he keeps fighting. The script throws it when either
side runs low (200 HAM) or after five minutes of fighting: the lines, the
explosion, then `die()`, which is also how he dies in normal combat. Next
test: does the grenade go off, and does he fall? Confirmed on the VM: the
lines, the explosion, and his corpse.

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
Quartermaster) as soon as the player arrives. A first run on the VM went
through, but the mission officer refused the first time: the gate waited
for the trainer room's narration. Gates are now set on every tick from the
room the player is in, whatever the script is doing, and a step the player
has gone past stops waiting (`docs/TUTORIAL.md`).

### 22. No welcome mail after the tutorial (and no mail from zones) — fixed

The original sent new players a welcome letter and a letter for their
starting profession (`newbie_tutorial/newbie_mail`). None arrived here.
`Tutorial::sendStartingMails` sends them when the player picks a
destination, to the ChatServer (`opIsmSendSystemMailMessage`), whose
`_processSystemMailMessage` looked up the recipient's name with a query
whose result nothing handled (`ChatQuery_QueryFirstName` had no case), so
the mail was dropped. Every mail a zone sends went the same way: bank wire
transfers (`sendBanktipMail`) and bazaar instant sales too. It now goes
through `ChatManager::sendSystemMailMessage`, the path the ChatServer's
own structure and auction mails use, which stores the mail and tells the
player if online. Also fixed: the mail leaked when the recipient was not
found, and `sendStartingMails` leaked its message object.

Then the mails arrived with an empty body. A system mail's body is a
reference into the client's string files, sent as binary out-of-band
data in the mail's attachments, and those were stored in a `utf8mb3`
`TEXT` column through a `TEXT` parameter of `sf_MailCreate`: bytes that
are not valid UTF-8 (`FF FF` in the header) are refused or mangled.
Attachments are now a `BLOB` (migration 005, the function, the schema),
and the ChatServer sends them as `_binary` literals. Player mail with
attachments (waypoints) went through the same column.

Still empty after that: the body itself was built wrong. `atMacroString`
writes the out-of-band package count as its internal counter, 0 for the
first package (bytes `00 00` where the client needs `01 00`), plus 8 stray
bytes at the end, so the client found no package to show. The newbie mails
and the bank wire mails now use `common::OutOfBand`, the builder system
messages use (and they display). Still on `atMacroString`: the bazaar
mails, which carry a waypoint `common::OutOfBand` can't add yet; they need
the same count fix or waypoint support in `common::OutOfBand`.

And reading them back: the database layer copied raw fields (`DFT_raw`,
the mail's attachments) with `strncpy`, which stops at the first zero
byte, and UTF-16 has one in nearly every other byte. No attachment ever
came back: not the system mails' bodies, not a waypoint a player attached
(seen on the VM: plain player mail worked, the reply's attachment was
missing). Raw fields are now copied whole, up to the field's size.

And storing them: `escapeString` ignored its length and passed the bytes
to the connector as a C string, so an attachment was stored as its first
byte. Shown with the database layer and MariaDB: the welcome mail's body
(124 bytes) came back as `01` and zeros; it now comes back identical. Only
the mail attachments pass bytes with zeros; for every other caller (names,
text) the result is the same as before. Mails stored before this fix stay
empty; new ones open whole.

Confirmed on the VM: a new character's welcome and profession mails show
their full text, and a player's mail with a waypoint attached arrives with
it. Still open: the bazaar mails (on `atMacroString`, see above) and the
`newbie_mail` prompt on arrival.

Still to add: the client's prompt for it, `system_messages:newbie_mail`
("You've received a message.  Press the letter-icon..."), on arrival.

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

### 23. Harvesting a creature corpse does nothing — fix needs testing

A Novice Scout (skill 31, `outdoors_scout_novice`) with loot rights gets
"harvest corpse" and harvest meat, hide or bone on a creature's corpse
(`AttackableCreature` radial), but `ScoutManager::handleHarvestCorpse` is
empty: nothing is harvested. The creature data names the resources (the
womp rat: `res_meat` Carnivore Meat, `res_hide` Bristley Hide) with
`harvest_qty` 0. The 2004 guide's creature tables give each creature's
meat, hide, bone and milk type and quantity; harvesting belongs with the
scout's review, and the quantities with the creature transcription.

Now (`ScoutManager::handleHarvestCorpse`): a Novice Scout with loot rights,
within 10 m, harvests the corpse once. Harvest meat, hide or bone takes
that part; Harvest corpse takes one the creature has, at random. The
amount is the creature table's quantity (migration 011:
`creature_meat_qty`, `creature_hide_qty`, `creature_bone_qty`) plus 1% per
point of the `creature_harvesting` skill mod, and earns as much scout XP
(`rules.harvest`, reconstructed). It goes into the inventory as the
planet's current spawn of that type (wild meat on Tatooine:
`meat_wild_tatooine`), topping up a container of the same resource first.
The radial's harvest meat/hide/bone entries now hang under Harvest corpse
(their parent index pointed at a missing entry). The greater desert and
variegated womp rats have no quantities yet (the 2003 table lists no
resources), so they yield nothing. Milking is not done.

### 26. Sampling into a full resource container records the wrong amount — open

`ArtisanManager` (sampling): when a sample overflows a container of the
same resource, the container is filled to its maximum and the rest goes in
a new one, but the database update writes the overflowing total, not the
maximum, so the container holds more than it can after a reload. Found
while writing the scout harvest, which does not share the bug.

### 27. Items can't be put in the bank — open

Found in the step 2 test pass (T40, Nov 2026). Joining a bank, depositing
and withdrawing credits work, but the bank terminal's item box (the safety
deposit container) does nothing: `TreasuryManager::bankOpenSafetyDepositContainer`
is empty. Pre-CU banks stored items as well as credits. Needs the bank's
container opened for the player and items kept in it, as an inventory is.

### 28. Missions were switched off — back on, needs testing

Found in the step 2 test pass (T33-T35, Nov 2026): the terminals answered
"Missions are currently disabled, do not report this as a bug". The
original team had made every mission handler (`OCMissionHandlers.cpp`)
send that message and return before the mission code, with no reason
recorded. They run again, with checks added where the code assumed a
terminal, mission bag, datapad or bank existed. Setting `Missions_Enabled`
to `0` in `config_server` switches them off again.

The first test crashed the zone at the first mission list: the mission
manager itself was never created, its `Init` commented out in
`WorldManager.cpp`. It is created again on every zone but the tutorial,
and the survey check that calls it is guarded. Mission names
(`mission_names`) exist for Tatooine only; the mission givers are still
the original placeholder list in `MissionManager.cpp`.

With the manager back, missions listed, showed their details and were
accepted to the datapad with a waypoint, but every distance was thousands
of kilometres. The random offset divided `rand()` by 32767, MSVC's
`RAND_MAX`; glibc's is 2147483647, so on Linux "50 to 500 m" reached about
29,000 km. It uses `RAND_MAX` now. The same search found the NPC choice of
deliver, entertainer and survey missions always rolling 1 and testing it
with `=` (so always the first NPC), loops that never ended when fewer than
two suitable NPCs were near the terminal, a division by zero for a
terminal without destroy missions, and shuttles' start-up timing always
0. All fixed.

Two more switched-off pieces:
- Looting with a destroy mission in the datapad hung the zone: what was
  left of the old "loot near the target completes the mission" code looped
  over the missions without ever moving past a destroy mission. Removed.
- Talking to a deliver or crafting mission's NPC did nothing: the check
  was commented out of the conversation handler. It runs again.

Destroy missions spawned nothing at their waypoint, so they could not be
completed. They now work as remembered from pre-CU: the waypoint is 20-60 m
from the target; within 128 m of it, the mission's lair spawns and the
waypoint moves to the exact location; destroying the lair completes the
mission and pays its owner. The lair does not respawn. The lairs are the
two fixed-position templates in `lairs` (1, womp rat rocks; 2, rill nest),
so this works on Tatooine only until step 3 adds lairs for every planet;
elsewhere destroy missions still have no target. Not done: credit for the
owner's group, and a mission lair matched to the mission's difficulty.
Missions also clear their scheduled check when deleted: an aborted recon
mission left the scheduler calling a deleted mission.

Confirmed on the VM (Nov 2026): a destroy mission's waypoint moved to the
lair as the player came close, the rill nest was there, destroying it
completed the mission and paid 3,546 credits. The test showed two lair
bugs, for every lair, not just missions' (fixed, to retest):
- Attacking a lair never called its creatures: `requestLairAssistance`
  and the creatures' lair-assist code were never called. An attacked lair
  now calls its living creatures that aren't fighting, as a pack does.
- Lairs took combat states (knockdown, stun, blind, intimidate, posture
  changes). Attackable objects (lairs, debris) take none now.
Retested on the VM (Nov 2026): the lair's creatures came to defend it, and
the combat music stopped once they were dead.

The next test (Nov 2026) found every terminal listing the same kind of
mission, and no deliver missions. Two causes (fixed, to retest):
- Crafting missions were given no title, description or creator. The
  mission delta sends only the fields that are set, so the client kept the
  previous list's text: crafting missions showed as destroy missions. They
  now take theirs from `mission_npc_crafting_neutral_easy` (50 missions).
- Deliver missions looked for two NPCs within 30 m of the player, which a
  terminal rarely has, and gave up. They now look within 1,500 m, as
  crafting missions do. The spatial index's range search turned out to
  stop at about 150 m (three grid cells) whatever range it is given, so
  deliver, crafting and entertainer missions now go through the zone's
  NPCs instead, taking outdoor townsfolk only. The log then showed 127
  candidates and still no mission: the loop that picks the two NPCs tests
  whether a position is still (0,0,0), and the position started
  uninitialised (glm does not zero a vec3), so it never picked one. It
  worked at the first terminal by luck. Crafting and entertainer missions
  had the same fault.
Confirmed on the VM (Nov 2026): a deliver mission listed in Bestine, picked
up and delivered. For later: its NPCs are any outdoor townsfolk, so they
don't fit the mission's text (an Imperial officer's cake went to a Jabba
thug); pre-CU deliver missions spawned their own NPCs for the mission.

The other kinds, tested in the step 2 pass (T43, Nov 2026):
- **Recon** (scout terminal): accepted, waypoint 50-500 m from where it was
  taken (inside the city, here); completed within 20 m of it (confirmed).
- **Crafting** (artisan terminal): works like a deliver mission, talk to
  the first NPC and then the second; nothing gives a schematic or checks
  that anything was crafted. Pre-CU, the first NPC gave a mission
  schematic and parts, and the crafted item was handed to the second. To
  build.
- **Survey** (artisan terminal): `checkSurveyMission` completes it when a
  survey of the named resource reads at least the mission's efficiency
  more than 1,024 m from the terminal. The first test asked for Class 1
  Radioactive, which wasn't spawned: the mission picked any mineral type.
  It now picks a mineral spawned on the planet (fixed, to retest). The
  efficiency asked for (30-70%) still ignores how rich the spawn is.
- **Entertainer** (entertainer terminal): accepted, performed at the
  waypoint, never completed. Switched off since the original import: the
  calls that start the mission's timer when a performance starts
  (`checkMusicianMission`, `checkDancerMission`) and the one that
  completes it when the timer buff ends (`missionCompleteEntertainer` in
  `Buff.cpp`) were commented out. Back on (fixed, to retest): performing
  within 20 m of the waypoint starts a 10-minute timer (music had 1
  minute), and when it ends the mission completes if the player is still
  performing there; otherwise it says so, and performing there again
  starts a new timer. The timer buff has no icon, so a system message now says when it
  starts. Missions in the datapad don't survive a server restart. The
  first retest ran 15 minutes without completing: the timer has icon 0,
  like other internal buffs, and `AddBuff` deletes a buff whose icon the
  player already has unless it is stackable, so the timer was thrown away
  at once. It is added as stackable now (fixed, to retest). The next retest
  started the timer, but the musician ran out of action ("You are too
  tired to continue playing music") in the same second the 10 minutes
  ended, and the mission, which required the player to be performing at
  that moment, didn't complete. Now stopping early cancels the timer at
  once (with a message), and at the end the mission completes if the
  player is still at the spot; a stop in the last two seconds still counts. The
  retest after that ran 11 minutes without completing: the buff scheduler
  runs a copy of each buff (`addBuffToProcess`), so the buff that ends is
  never the mission's own and the pointer check never matched (probably
  why the original authors commented it out). Asking the timer whether its
  time was up didn't work either: the global tick it measures against
  counts a once-a-second task and falls behind real time, so at the real
  10 minutes it still had time left, and an abort later completed the
  mission and then crashed (the abort's buff removal ran the completion,
  which deleted the mission the abort was still using, and the buff was
  deleted twice). Now the ending copy is matched to the mission's timer by
  its start tick and length, the early-stop grace uses real time, and an
  abort detaches the mission from its timer before removing the buff and
  no longer deletes it itself. Confirmed on the VM (build 2fcf713): the timer started at
  23:02:24 and the mission completed at 23:12:24, paying 1,458 credits.
- Entertainer missions put their spot on a random outdoor townsfolk NPC
  (the generator borrows an NPC's position), so every one lands on or
  next to someone. Pre-CU sent the entertainer to a venue or a spot in
  town ("the patrons at this establishment"). With the deliver missions'
  NPCs, for later.
- Survey missions showed "Unknown Planet" in the mission list's Start
  column: they set a start but no destination, which the list shows. The
  destination is now the spot where the mission was taken; the list shows
  "0m to the N" now. Confirmed on the VM: a survey mission completed. For
  a later pass: give each survey mission its own distance from the start
  (the 1,024 m now fixed in code), and show it in that column.
- Aborting an entertainer mission part-way no longer crashes (confirmed).
The accepted mission's datapad entry (description, creator, payment,
difficulty) was filled in; the empty one seen earlier was from a mission
generated by the faulty code.
A mission that can't be generated is now left out of the list (an old
refresh count) instead of showing stale text.

### 31. Harvesters extract per hour, not per minute — to check

Found in the step 2 test pass (T29, Nov 2026): after about 20 minutes, a
harvester with a rate of 1.87 held 0.65 units, so its hopper still looked
empty. `sf_HarvestResource` adds `rate / 60` every minute, which makes the
rate units per hour. Pre-CU harvesters' extraction rates were, as far as
we remember, units per minute, which would make harvesting 60 times too
slow. To check against a source before changing.

### 29. Houses could not be renamed from their terminal — fixed

Found in the step 2 test pass (T31, Nov 2026). The house terminal's
Structure Management menu had no way to name the house (harvesters and
factories have Set Name on their own radial), and `/nameStructure` only
took the building itself as target, which can't be targeted from inside.
The terminal now offers Name Structure, and `/nameStructure` also works
with the terminal targeted from the same room. The command also read the
typed name with `sscanf` into an unsized buffer; it now only checks its
length (the name is entered in the rename window).

Confirmed on the VM (Nov 2026): a house renamed from its terminal.

### 30. No sign outside a placed house — open

Found in the step 2 test pass (T31, Nov 2026). Pre-CU houses had a sign by
the door showing the house's name. The code that made one is commented out
in `StructureManager::_handleStructureObjectTimers`: it put a street sign
at a fixed offset from the house whatever its rotation, and the sign was
not saved, so it would be gone after a restart. A proper sign needs each
house type's sign position and model, saved with the house.

### 32. Zone and chat servers crashed after "Lost connection to MySQL server" — fixed, to retest

The chat server crashed overnight (Nov 2026), and a ZoneServer crashed
when a manufacturing schematic was made for a factory (T30). Both
backtraces end in `Database::process` freeing a query's result, the zone's
inside `mysql_stmt_close` in libssl after "Lost connection to MySQL server
during query".

The cause was ours, from the change to prepared statements (#24): each
database worker thread has its own connection, but results are freed on
the main thread after the worker has moved on to its next query. Freeing
a prepared statement's result sends the server a close over that
connection, so two threads used one connection at once, which garbled it
and crashed. Plain queries' results don't talk to the server when freed.
A prepared statement's result now keeps its worker, and its connection,
until it is freed, as multi-result (stored procedure) results already did.

Retested on the VM (Nov 2026): all the servers ran through a day of
testing and an idle afternoon without a crash.

### 33. Chat server crashed when a harvester's maintenance ran out — fixed, to retest

Found on the VM (Nov 2026): the chat server crashed in
`StructureManagerChatHandler::handleDatabaseJobComplete` after logging
"Out of range value for column 'condition'". A harvester had run out of
maintenance and its owner's bank couldn't cover it, so
`sf_HarvesterUseMaintenance` lowered the structure's `condition` below
zero. The column is unsigned, so the update failed, the query returned no
result, and the handler read the row count of a result it didn't have.

- `sf_HarvesterUseMaintenance` stops `condition` at 0 (run
  `mmoserverdb/setup.sh update-procedures`).
- The chat server's structure handler skips a failed query, as the
  zone's handlers that use prepared statements already do. Other handlers
  that don't check for a failed query remain; each would crash the same way
  if its query failed.

The log also showed "Record has changed since last read in table
'structure_attributes'": MariaDB 11.8's snapshot isolation refusing an
update to a row another server changed in the same moment. That query
fails and runs again at the next maintenance interval.

### 34. Factories count condition the other way — open

Found while fixing #33. `sf_FactoryUseMaintenance` treats a structure's
`condition` as damage, rising to the maximum ("condition is actually
damage"), while the C++ (`PlayerStructure`), the chat server's mails and
`sf_HarvesterUseMaintenance` treat it as health falling to 0. A factory
left without maintenance would show as healthier as it decays, and be
condemned at full condition. To fix before relying on factory decay.

### 35. Character select shows a galaxy as online while its zones load — open

Raised in the step 2 test pass (Nov 2026): the character select screen's
server status reads online as soon as the login server is up, even while
the zones are still loading and a login would fail or wait. It should say
the galaxy is loading until the zones are ready. For later.

### 37. OK on a list window with nothing selected crashed the zone — fixed

Found in the step 2 test pass (Nov 2026), teaching another character: OK on
the teaching window with no skill picked sent row -1, and
`PlayerObject::handleUIEvent` looked it up unchecked (`std::out_of_range`,
the ZoneServer aborted). About fifteen list windows index their rows the
same way (teaching, travel and ticket collectors, insurance, dances and
music, the character builder, the factory's schematic list). `UIListBox`
now treats OK with no valid row as Cancel, before any handler sees it,
except for the factory's Remove Schematic (a third button that needs no
row) and the structure status window (its OK is Refresh).
Confirmed on the VM (Nov 2026): OK on the teaching window with nothing
picked just closes it.

### 38. Image design session crashed the zone on a new hairstyle — fixed, to retest

Found in the step 2 test pass (T38, Nov 2026): the changes went through,
then the ZoneServer aborted on `assert(false)` in
`ContainerManager::registerPlayerToContainer`, from
`EntertainerManager::applyHair`. The old hair was removed with
`destroyObjectToRegisteredPlayers`, which looks the object up in the
world's object map; hair isn't there, so no watcher was unregistered from
it. The old hair was then deleted, leaving the watching players with a
dangling pointer, and the new hair, often at the same address, was
"already known". `applyHair` now unregisters the old hair from every
watcher (which also destroys it on their clients) before deleting it.

The retest showed the fee never worked: with a fee set, the designer's
Commit stayed greyed out, because the customer's offer never reached the
designer (their window showed Offered Credits 0, and the client only
enables Commit once the offer covers the fee). The server sent every
update to the customer, the customer's own included, and built it from the
receiver's change list, which is kept on the customer. Updates now go to
the other side of the session, built from the customer's changes (fixed,
to retest). Without a fee, the session completed and the credits offered
were paid.

Retested on the VM (Nov 2026): a new hairstyle with a 50-credit fee; the
offer showed on both sides, the designer committed, the customer accepted,
the hair changed, no crash, and the designer got 50 Image Designer
experience. Still open, minor: opening a second session on the same
customer straight away shows them bald in the designer's preview, with
"You are already in the process of an Image Design action". The new hair
reuses the old hair's object ID (customer + 8), which the designer's
client may still hold as destroyed; to look at if it matters.

### 39. Structures are barely charged maintenance — open

Found in the step 2 test pass (T30, Nov 2026): a factory ran for hours,
filling four crates of small glasses, and its maintenance pool stayed at
the 50 credits put in ("50/hr" on the status window). The chat server
charges every structure (houses, harvesters, factories) hourly with
`sf_HarvesterUseMaintenance`, which takes `maint_cost_wk / 336` each time,
as if the rate were per week and charged every half hour; the status
window shows the same column as a rate per hour. At 50 that is about 0.15
credits an hour, which rounds to nothing. `sf_FactoryUseMaintenance` and
`sf_FactoryUsePower` are never called, so factories use no power either.
The maintenance mail's 24-hour throttle in the chat server reads
`lastMail < now + 24h`, always true, so that mail is never sent.
To settle from the guides: whether the rate is per hour or per week, how
often it is charged, and whether factories use power. Then charge each
kind of structure with its own function. With #34 (factories count
condition the other way) and #36 (ingredients).

### 40. Idle servers use a lot of CPU — open

Found on the VM (Nov 2026, `sar` over a night with no players): about 25%
of four cores went to kernel time and 9-10% to user time, flat, with the
servers running under `debug -g` and nobody logged in. Most likely the
servers' main loops poll their sockets with very short sleeps instead of
waiting for packets. To measure per server (`pidstat -u -p <pid> 60`) and
fix after the step 2 pass. Not the cause of the VM freeze on 18 November:
memory, swap and CPU were flat up to the last sample at 04:00, so it is
put down to the host PC (the VM runs in VirtualBox on it). A long run on a
rented cloud VM would rule the host out (ROADMAP, Parked).

### 41. Crafted values didn't become the item's stats — fixed for weapons

Found in the step 2 test pass (T27, Nov 2026): two CDEF carbines, one
assembled only and one after a failed experiment. Each tooltip shows the
template's stats (Damage 25-50, Attack Speed 3.5, Condition 100/100),
identical on both, and below them the crafted results as extra attributes
(Minimum and Maximum Damage, Attack Speed, Wound Chance, Condition
1000/1000, the special move costs), which differ. So crafting and
experimenting compute values but don't apply them to the item's real
attributes; combat most likely uses the template's. The crafted values
also look unscaled (Attack Speed 35.78 for 3.578 s). To fix: map each
crafted attribute onto the item's own (damage, speed, range, wound chance,
condition, costs) with its scale, and drop the duplicates. To check from
the guides: a plain experiment failure lowered the values here; pre-CU, as
far as we remember, only a critical failure did.

Experimenting itself works: with Engineering I and the matching station,
the experimentation step came up after assembly.

Fixed for weapons: when a prototype is made, and when a manufacturing
schematic's template item is stored (factories copy it), `applyCraftedStats`
copies each crafted value onto the weapon's own attribute, the one
`CombatManager` reads, in its units (attack speed in tenths of a second
becomes seconds), and removes the crafted duplicate (new
`Object::removeAttributeIncDB`, which also drops it from the display order
and the item's row): minimum and maximum damage, wound chance, attack
speed, the range modifiers, the three attack costs, and unit integrity as
the condition. Attack speed and the attack costs are lower-is-better:
experimenting now moves them down their range (`modifyAttributeValue`).
Other kinds of crafted item (armour, food, clothing...) keep their crafted
attributes as before; their mapping is still to do, kind by kind. Items
crafted before this keep both sets.
Confirmed on the VM (build b0ff0c5): an experimented CDEF carbine shows
one set of stats (damage 22-45, speed 4.14, wound 4.33, ideal range 43,
costs 9/20/9, condition 1005/1005). Still shown twice: the item's
condition (the 100/100 at the top comes from the object itself, the
1005/1005 below from the attribute); to make one follow the other.
A third carbine, after a great success, confirms it: its crafted values
rose (Maximum Damage 46.71, Minimum 22.85, Unit Integrity 1012.68 against
43.11, 21.05 and 952.68 without experimenting) while the template block
stayed at Damage 25-50, speed 3.5. Its crafted Attack Speed also rose
(37.46 against 35.78); if that is the time between attacks, a success
should lower it, so the attribute may be the wrong way round too.

### 42. ConnectionServer crashed overnight on a missing message — fixed, to retest

Found on the VM (Nov 2026), after a night idle with a client that had
connected and reconnected: SIGSEGV in `Message::ResetIndex` (this=0x0)
from `Service::Process`. The loop asks the session how many messages are
waiting and then takes that many; the count read the queue's size with no
lock while the socket thread was adding to it, and `getIncomingQueueMessage`
checked the size before taking the lock, so the loop could take a message
that wasn't there. Both now hold the session's lock, and the loop stops at
an empty queue. The same code runs in every server.
Not retested: overnight runs on the test VM have had mixed results that
look like the host (#40), so long runs wait for a separate machine
(ROADMAP, Parked).

### 43. Camps have no terminal, and their layout and chair look wrong — open

Found in the step 2 test pass (T39, Nov 2026): a basic camp placed with
its tent, torches, decorative campfire and chair, but no camp terminal,
so it can't be checked or disbanded. The code is there (`CampTerminal`,
with Status and Disband, spawned by `ScoutManager::createCamp` from the
camp's rows in `structure_item_template` marked as a terminal); the data
isn't: no camp has a terminal row, and `terminal_types` has no camp
terminal. To fix with a migration: the camp terminal's template (its name
from the client: `tools/client/tre.py list <client>
'object/tangible/terminal/*camp*'`) in `terminal_types`, and a terminal
row in each of the six camps' layouts. The layout itself looked off (a
basic camp lists only the campfire and the chair; the furniture positions
were server data pre-CU, so a screenshot of each camp type is the best
source), and the character sat on the chair oddly (it may be placed as a
decoration rather than as a seat).

A look through the client found no template named for a camp terminal.
To settle before building: what pre-CU camps offered for status and
disbanding (a terminal under another name, or the camp's own menu).

The Tatooine guild hall (T32) placed with no guild terminal; the generic
one (after #48) has a Guild Management Terminal. Guilds themselves aren't
built (roadmap step 7), so the terminal does nothing yet.

### 44. A vehicle wouldn't move after mounting again — fixed

Seen twice in the step 2 pass (Nov 2026), the second time after sitting
in a camp: the player mounted the speeder and it stayed put, with the
HUD's posture icon not standing. `VehicleController::MountPlayer` set the
riding state but left the posture as it was, and the client doesn't drive
a seated or kneeling rider. Mounting now stands the player up first.

### 45. Placing an instrument crashed the zone — fixed, to retest

Found in the step 2 test pass (entertainer missions, Nov 2026): using a
Nalargon from the inventory (it places a temporary copy) logged "You have
an error in your SQL syntax ... near 'swganh.JOIN attributes'" and the
ZoneServer crashed in `FactoryBase::_buildAttributeMap`. The
non-persistent item factory's query read `INNER %s.JOIN attributes`
instead of `INNER JOIN %s.attributes`, so it always failed, and the
attribute map read the missing result. The query is fixed, and
`_buildAttributeMap` (shared by every item factory) loads the object
without its defaults if a query fails, instead of crashing.

The instrument's default attributes in the database are placeholders:
condition "###/###" and an owner of "Players Name here (Needs code)". Small
data to fix with a migration.

### 47. A musician doesn't hear their own music — open

Found in the step 2 test pass (Nov 2026): playing the Nalargon, the
musician's own client plays no music; another character listening hears
it. Pre-CU, the performer heard their own performance. Most likely the
performer's own client isn't sent what the listeners get (the performance
id, or listening to themselves). To look at.

### 48. A generic guild hall wasn't placed, and its lots were lost — fixed

Found in the step 2 test pass (Nov 2026): placing a generic guild hall
with 5 lots free built nothing and left the character with 0 lots.
- `StructureManager::HandlePlaceStructure` listed the Corellian, Naboo and
  Tatooine guild hall deeds but not the generic one, so its placement
  fell through and nothing was built.
- The deed took the lots when placement mode began, before anything was
  placed (the client doesn't say when the player presses Escape), so any
  placement that didn't finish, or was refused, or was on the wrong
  planet, lost them.
The generic guild hall is placed like the others now; the deed only checks
the lots, and they are taken when the structure is placed and given back
if placement fails there (no height for the spot, a deed with no
placement, a city hall without the skill); pressing Escape out of a
placement now keeps them (confirmed). Lots are counted from the
structures owned at login, so relogging gave back the lost ones anyway.
The first retest crashed the zone on placement: the command map binds
`HandlePlaceStructure` to `gStructureManager` before the manager exists,
so the handler runs with a null `this`; the new lots check called
`getDeedData` on it. It goes through `gStructureManager`, as the rest of
the handler does.
Confirmed on the VM (build b0ff0c5): the generic guild hall placed, with a
Guild Management Terminal inside.

### 46. /tip bank refused with "bank" first — fixed, to retest

Found in the step 2 test pass (Nov 2026): `/tip bank Obitwo 100`, and
`/tip bank 100` with the player targeted, both answered "You may only /tip
or /tip bank to other players". The handler only recognised "bank" as the
last word (`/tip Obitwo 100 bank`, pre-CU's own order). It now accepts it
anywhere.
Confirmed on the VM: `/tip Obitwo 100 bank` sent the credits to the other
character's bank.
Storing the speeder and calling it again freed it (the player was
standing by then), which fits.
The retest disproved that part: the client (the server's command check)
refuses to mount while kneeling or sitting, correctly, but after standing
up and mounting, the speeder still didn't move; storing and calling it
again frees it. Still open. A refused state change (such as becoming a
rider) sent its "You cannot transition from this ... state" message to no
one; it now goes to the player and is logged with the states, to find
what blocks it.
A cleaner reproduction followed: call the speeder, mount and drive,
dismount, mount again, and it won't move; nothing was refused. Mounting
sets the vehicle's MountedCreature flag directly and sends it; dismounting
cleared it through `removeActionState`, which leaves it set if the
removal is refused (that was only a debug log line, a different one from
the search). With the flag still set, the second mount changed nothing the
client could see. Dismounting now clears the flag directly and sends it,
mounting clears a stale one first so the change is always seen, and a
refused removal is logged and told to the player. Confirmed on the VM:
mount, dismount and mount again, and the speeder drives each time.

Also seen, to check against the guides before changing:
- `/imagedesign` opens the window with no prompt to the other player.
- Changes can only be committed inside an image designer's tent. That is
  the client's rule; the server doesn't check it.
- "Time until committable" reads Ready at once: the session timer isn't
  sent.

### 36. Factories never got their schematic on Linux — fixed, to retest

Found in the step 2 test pass (T30, Nov 2026): a factory said "Schematic
%TT has been inserted into the station", started, and made nothing; in the
database both factories had `ManSchematicID = 0` and were switched off.

The factory commands built their SQL with `%I64u`, Microsoft's printf
notation for a 64-bit number. glibc reads it as a 32-bit `%u` 64 characters
wide, so only the low half of each ID reached the database: inserting a
schematic updated no factory, and starting or stopping one changed nothing.
Sixteen uses, all now `PRIu64`: the factory commands (start, stop, insert
and remove a schematic, the schematic's details, discarding a hopper),
making a house private or public, and saving where a moved item or
resource container was put (`OCCommonHandlers.cpp`), so items moved in
the world may not have kept their place across a restart on Linux.

Also fixed: the message now names the schematic (`%TT` was never filled).

Confirmed on the VM (Nov 2026): the schematic linked, the factory ran and
its output hopper held a crate of 4 small glasses a few minutes later.

Opening the schematic slot of a factory that holds a schematic then
crashed the client, with no crash log. That window adds a third button
(Remove Schematic), and the buttons' PackSize and PackLocation properties
were declared with one value but written with one per button number (two
for Use, three for Remove), so the client read the rest of the window out
of step. Never seen before because no factory had held a schematic on
Linux. Each property now has its one value. Confirmed on the VM: the
window opens, and Remove Schematic returns the schematic to the datapad.

Removing the schematic didn't stop the factory: the zone kept it running
(the database switched it off at its next step, silently, for want of a
schematic). Removing it now stops the factory as Stop Manufacturing does
(fixed, to retest).

Still open:
- **Factories don't use ingredients** — built, to retest. A manufacturing
  schematic's ingredients are now stored when it is made (migration 018,
  `manufactureschematic_ingredients`: per slot, the resource or the
  component's item type and, for an identical component, its serial, and
  the amount one item takes). `sf_FactoryIngredients` checks the input
  hopper before each item and takes the ingredients after the item is
  made; without them the factory stops (return 4) and the owner gets a
  "Factory stopped" mail, as for a full output hopper (2). List
  ingredients on the factory's menu shows them. Limits: components are
  single items in the hopper (not crates), an emptied resource container
  stays at 0 for the player to take out, and schematics made before this
  have no ingredients recorded and keep producing for free. The zone
  learns that the database stopped a factory when its menu is next
  opened (below). First test (Nov 2026): a new schematic recorded its
  ingredient and List ingredients showed it ("100 units of Osheig"), but
  some resource stacks, all gemstones and every newly split stack, were
  refused by the input hopper. `Object::mCapacity` was never initialised,
  and resource containers never set it, so it held garbage; the transfer
  check refuses an item whose capacity isn't below the container's as
  "too bulky". It starts at 0 now. Inserting a schematic also printed
  debug text about schematic and factory masks (tested with `&&` for
  `&`); that is a log line now, and the match isn't enforced yet. The retest worked in the database: one large glass made, the
  100 Osheig taken (the container at 0), then the factory stopped and
  mailed its owner. The zone still showed 100 in the hopper:
  `ResourceContainer::upDateFactoryVolume` took a `BString` where
  `TangibleObject`'s takes a `std::string`, so it never overrode it and the
  amount read back on opening the hopper was dropped (taking the stale
  stack out could have written 100 back). It overrides it now. And the
  factory's menu re-reads its running state from the database each time
  it opens (the chat server stops it there), updating the work animation
  if it changed; the menu just opened can still be one step behind.
- The factory functions turn a factory off without a word when something
  they need is missing (return 3), and the chat server ignores the return
  code.

### 24. A cloned player stays a corpse — fixed

Killed by womp rats near Bestine (three incaps inside the reset time, the
`Player_Incapacitation` rule), the player cloned in Bestine but lay there as
"corpse of ...", with empty HAM; `/stand` and `/kneel` answered "You cannot
... while Standing". The Dead posture refuses every transition out of it
(its deny list holds Dead itself) and locks the state layers, and
`PlayerObject::clone` never left it, so `resetProperties` could not stand
the player up and the client kept getting posture Dead. HAM was never
refilled either. The clone now unlocks the layers, sets posture Upright and
locomotion Standing, and fills health, action and mind to their maximum less
wounds (`Ham::restoreMainPools`). Tested on the VM: the clone stands with
full HAM. The clone also stayed in combat: death's clearing of the action
states is refused by the same lock (`ActionState::CanTransition` checks
`blockAction`), so the clone now clears them too (needs testing). The clone's 100 health
wound was lost: a wound can't exceed the pool's current points and death
leaves health at 0, so the pools are now filled before the clone's wounds.

### 25. Entertainers never heal; medics' healing skill does nothing — fixed

Found while adding bleeding:

- `EntertainerManager::heal` returned when the entertainer was not in a
  cantina, *or* not in a camp, *or* not in a player structure: always.
  Nobody watching a dance or listening to music had a mind wound or battle
  fatigue healed, nor did the entertainer. It now heals in a cantina, camp
  or player structure; battle fatigue still only in a cantina.
- `PlayerObject::getPlayerLocation` called a player outside a building a
  camp when they were *not* in a camp region; now when they are.
- The heal's audience loop only advanced past players: a non-player in the
  list would have looped forever.
- Heal amounts used integer division, `base * ((100 + mod) / 100)`, so any
  skill mod under +100 did nothing: the entertainers' wound and shock
  healing mods and the medics' `healing_wound_treatment` (Tend Wound and
  Heal Wound). The stimpack heals (`HealDamage`, `HealDamageRanged`) have
  the same bug; left for the review of the medic professions.
- `MedicManager::CalculateBF` tested battle fatigue over 500 before over
  750, so the "high" message never showed.

Tested on the VM (Nov 2026): an entertainer dancing in a Bestine cantina
healed a watching brawler's mind wounds and battle fatigue, and a medic's
`/tendwound health` and `/tendwound action` healed wounds.

Still open: the entertainer group-buff code (`EntertainerManager.cpp`, the
two `loc != ... || loc != ...` checks next to `dance_watch_stop_self` and
the music equivalent) has the same always-true location test and drops
buffs; buffs are their own pass.
