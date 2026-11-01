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
| T5 | Tutorial | Corpse looting; **Loot all** locked up the zone and ran it out of memory [10] | Kill the bandit, then on the corpse: (a) Loot, drag the melon to your inventory; (b) on another run, Loot all. Watch `tail ~/zone-mem.log` | Items arrive in your inventory; the corpse is empty; you leave combat; chat works; the tutorial zone stays near 120 MB |
| T7 | World | Combat creatures never spawned (lairs) [5f] | On Tatooine, go to the lair area around **x −2200, z −4700** (about 1.4 km south-west of Bestine; `/waypoint -2200 -4700`) | Creatures from the lairs appear; attack one, it fights back a little and dies lying down |
| T8 | Resources | A sample continued after its resource expired | Start sampling a resource, then expire it: `sudo mariadb swganh -e "UPDATE resources SET spawn_end=NOW() WHERE name='<name>'"`. Keep sampling for up to 10 minutes | Sampling stops with "no resource here" once the rotation runs |
| T9 | Text | Client text garbled on Linux [2a] | Say something in spatial chat with punctuation and numbers; use an emote; examine an item | Text appears as typed; emote plays; attributes show |
| T10 | Stability | Early disconnect in the tutorial [1] | Stand idle in the tutorial for 5 minutes | Still connected (not seen recently; may be fixed by the reconnect work) |
| T11 | Stability | Overnight run with players | Leave a character logged in (not idle-kicked) on Tatooine overnight | All servers still running in the morning (`./server.sh status`) |

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
| Stability | Out-of-memory kill of the tutorial zone [10] | Reproduced with heaptrack: Loot all on the bandit's corpse; fixed, see T5 |
| Stability | Idle memory | 22 hours with no client: every server at its fresh size (tutorial 116 MB, Tatooine 274 MB, ConnectionServer 137 MB), heaptrack peaks only startup allocations |

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
