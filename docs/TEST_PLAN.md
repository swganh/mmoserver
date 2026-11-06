# Test plan

Client tests for everything fixed so far, so each fix is confirmed with a
real client before moving on to the Windows build. Details of each issue are
in `docs/KNOWN_ISSUES.md` (numbers in brackets).

**Status:** ✅ confirmed with a client · 🧪 to test · ⚙️ server-side
check only (no client test needed).

## Before testing

```bash
cd ~/mmoserver && ./server.sh stop && git pull && ./build_server.sh
```
```bash
sudo ./mmoserverdb/setup.sh update-procedures
```
```bash
./server.sh debug -g
```

Logs are in `~/mmoserver/build/linux-debug/bin/logs/<server>.out`. If a
server crashes, gdb stops it and the backtrace is at the end of its log.

## To test

| # | Area | What was wrong | How to test | Expect |
|---|---|---|---|---|
| T3 | Login | Refusals were silent [6] | If anything is refused at the galaxy screen | `connection.out` has a `Refusing account N: <reason>` line; send it to me |
| T4 | Script events | New | `sudo ./mmoserverdb/setup.sh update`, then on Tatooine: log in, walk into Mos Eisley, talk to an NPC and pick an option, use a radial menu item, kill a creature, log out. Watch `tail -f tatooine.out \| grep 'event:'` | one `event:` line per action, with the player's id as `source` |
| T5 | Scripts | `print` output was lost [15] | Start Tatooine | `tatooine.out` has `[script] WeatherController v.0.2` and `[script] event_logger: listening to 9 events` |

## Confirmed

| Area | What was wrong | Confirmed by |
|---|---|---|
| Install | Database install, collation errors on MariaDB 11.5+ | Installed on the Ubuntu 26.04 VM with MariaDB 11.8 |
| Install | No resources on a fresh install [5a] | Survey found resources after `spawn-resources` |
| Install | Stored procedures could not be reloaded | `update-procedures` installed the rotation procedure on the VM |
| Launcher | `server.sh debug -g` stopped at gdb's debuginfod prompt | Servers run under gdb in tmux |
| Galaxy | Galaxy stuck on "Loading..." (ChatServer race) | Galaxy online |
| Login | LoginServer crashed after a character was deleted | Deletes work |
| Login | Quick reconnect after killing the client refused, at login and at the galaxy [6] (T2) | Killed the client at profession selection, logged straight back in and created a character; `connection.out`: `Account 31 connected again; replacing its previous connection`; no `Login failed` |
| Login | Characters with a surname couldn't be deleted (T4) | `delete First Last` on the fixed build |
| Login | No character creation after a delete until logging in again [6] (T1) | Deleted and created twice without logging out; both loaded into Tatooine |
| Login | Characters with a surname couldn't be deleted | Typing the hidden trailing space deleted one (diagnosis); see T4 for the fixed build |
| Stability | Zone crash on reconnect [2] | Disconnect and reconnect, no crash |
| Stability | Character switching after a disconnect hung [2c] | New character transferred to Tatooine |
| Stability | Servers crashed after a long idle period [2d] | Overnight: ConnectionServer 9, Tatooine 16, tutorial 6 reconnects, all running |
| Stability | Idle client never cleaned up | `Player left` after the client timed out overnight |
| Tutorial | Skipping the tutorial crashed zones (garbled starting city) | Skipped to Tatooine |
| Tutorial | NPCs inside buildings invisible [5, 5b-2] | NPCs in every room, item room officer visible |
| Tutorial | "Out of range" talking to any NPC indoors | Conversations work |
| Tutorial | Crash at the item room drum ("open me" fly text) | Got past the item room |
| Tutorial | Debris had no Attack option, and stayed after dying [4] | Debris attackable and destroyed |
| Tutorial | Bandit never spawned [5f]; killed creatures stayed standing | Bandit fights, collapses when killed, XP awarded |
| Tutorial | Ending (Quartermaster, travel terminal) | Travelled to Tatooine from the end of the tutorial |
| Resources | Survey and sample | Resources found, samples taken at good concentrations |
| Resources | Resources never rotated [5b] | Expired resource replaced while running (`Replaced 1 expired resources`) |
| World | Tatooine NPCs | Patrol NPCs around Bestine |
| World | Killed creatures stayed on screen (T6) | The bandit's corpse disappeared after a few minutes |
| Tutorial | Looting the bandit; Loot all locked up the zone [10] (T5) | Loot shows the corpse's items and they can be taken; the zone stays responsive and near 120 MB. Still in combat afterwards: see KNOWN_ISSUES 5i (`/peace` works around it) |
| Resources | A sample continued after its resource expired (T8) | Expired by hand while sampling: gone from the survey list, sampling stopped |
| Text | Client text garbled on Linux [2a] (T9) | Chat with punctuation and numbers |
| Stability | Early disconnect in the tutorial [1] (T10) | Idle in the tutorial, still connected and loaded |
| Stability | Out-of-memory kill of the tutorial zone [10] | Reproduced with heaptrack: Loot all on the bandit's corpse; fixed, see T5 |
| Travel | Tickets to "Nym's Stronghold" never created [5k]; boarding a cross-planet shuttle did nothing [5l] | Bought a Bestine Starport to Nym's Stronghold ticket and landed on Lok |
| Stability | Overnight run with a player (T11) | Character logged in on Lok overnight with tutorial, Tatooine and Lok zones under gdb: all seven servers running in the morning, the player still in Lok's saves, no crash, zone memory flat at its fresh size (tutorial 106 MB, Tatooine 274 MB, Lok 239 MB) |
| Windows | Native build | Windows 11, Visual Studio 2022 (MSVC 14.44): `build_server.ps1 -Test` built everything with vcpkg, 134/134 tests pass |
| Windows | Servers on Windows | All servers started with `server.ps1` against the database on the Linux VM; logged in and played in Bestine on Tatooine (after fixing the grid crash, KNOWN_ISSUES #11) |
| Linux | No regression from the Windows changes | `windows-dev-env` built and played on the Ubuntu VM |
| Stability | Idle memory | 22 hours with no client: every server at its fresh size (tutorial 116 MB, Tatooine 274 MB, ConnectionServer 137 MB), heaptrack peaks only startup allocations |

## Not implemented yet

| Test | Why |
|---|---|
| T7, lair creatures on Tatooine | No lair ever spawns: their state machine is stripped (KNOWN_ISSUES 5j). Deferred to the combat and creature AI review |

## Known open issues (not tested)

Tracked in `docs/KNOWN_ISSUES.md`; not expected to pass yet:
servers listening on all interfaces [3], positions inside buildings
mirrored [5c], low sampling yield at low concentration [5d], tutorial
prompts lagging behind [5e], no debris explosion [5g], "Too many commands
queued" [5h], unhandled opcodes [7], startup data warnings [8], garbage
"MessageFactory Heap" log lines [9], creature AI largely missing.

## Recording results

Tell me the T number and what happened. For a failure, the end of the
relevant log helps most:

```bash
tail -n 60 ~/mmoserver/build/linux-debug/bin/logs/<server>.out
```
