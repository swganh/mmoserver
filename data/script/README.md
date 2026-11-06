# Scripts

Lua 5.1 scripts run by the zone servers. The C++ servers are the engine
(network, world, movement, combat rules, persistence); scripts say what
happens; tables in the database hold the data (items, loot, trainers, spawn
tables). The plan and the reasoning are in `docs/SCRIPTING.md`.

| Folder | What goes there | Started by |
|---|---|---|
| `lib/` | Shared modules for other scripts (`require 'script/lib/name'`) | `require` |
| `world/common/` | Scripts every planet runs (weather, login messages) | `config_zone_scripts` |
| `world/<planet>/` | One planet's world scripts (ambient life, events) | `config_zone_scripts` |
| `tutorial/` | The new-player tutorial | the tutorial zone, per player |
| `commands/` | Hooks for player commands (`/burstrun`) | `command_table.scripthook` |
| `ai/` | NPC and creature behaviours (patrol, formation, lair defence) | other scripts |
| `spawns/<planet>/` | Static NPCs, patrols and spawn regions | `config_zone_scripts` |
| `conversations/` | Dialogue trees for quest givers and named NPCs | (planned) |
| `quests/<planet>/` | Quest chains | (planned) |
| `themeparks/` | Theme park chains (Jabba, Imperial, Rebel, Nym) | (planned) |
| `missions/` | Mission terminal templates | (planned) |
| `dev/` | Tests and experiments. Not for live servers | `config_zone_scripts` |

Only folders with scripts in them exist yet; add the others when their first
script arrives.

## Conventions

- File and folder names are lower case with underscores:
  `world/tatooine/mos_eisley_cantina.lua`.
- Paths in the database and in `require` start at the server's working
  directory: `script/world/common/weather.lua`, `require 'script/lib/npc'`.
- Moving or renaming a script that the database names needs a migration
  (`mmoserverdb/migrations`) that updates the row, so existing servers follow.
  CI checks that every path in the database exists.
- Content comes from the client's own files and public material from the
  time, never from the leaked SOE server source.
