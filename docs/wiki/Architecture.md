How the parts of MMOServer fit together. The same on Linux and Windows.

## The servers

A galaxy is several programs working together:

| Server | Role | Port |
|---|---|---|
| **ConnectionServer** | Holds every client connection and routes messages between clients and the other servers. Start it first | 44991 (clients), 5000 (servers) |
| **LoginServer** | Checks accounts and lists the galaxy and characters | 44990 |
| **ChatServer** | Chat, mail, groups, friends, the bazaar | 5200 |
| **PingServer** | Answers the client's latency checks | 44992 |
| **ZoneServer** | One per planet (`--ZoneName tatooine`): the world, players, NPCs, combat, crafting | 5001-5011 |

A login goes: client → LoginServer (account, character list) → client →
ConnectionServer → the zone the character is on. Travelling to another
planet hands the player from one zone to the other through the
ConnectionServer.

The servers talk to each other and to clients over the same UDP protocol
(the original SWG one). The servers other than the PingServer read and
write the database directly.

## The database

MariaDB. Almost everything is in one database, `swganh`: accounts,
characters, items, resources, the game data, server settings
(`config_server`, `config_zone_scripts`) and the stored procedures the
servers call. The installer creates four more for the original project's
layout: `swganh_logs` (account and server accounting logs), `swganh_config`
(the schema version) and `swganh_archive` and `swganh_astromech` (empty).

See [[Database]] for installing and managing it.

## Files

```
data/config/        server settings (one .cfg per server, general.cfg for all)
data/script/        Lua scripts (tutorial, world scripts)
data/heightmaps/    planet terrain, downloaded on demand
mmoserverdb/        database schema, data, procedures and setup.sh
src/                the C++ source, one folder per server or library
tools/              build helpers, heightmap checksums, vcpkg ports
```

A build puts the servers in `build/<preset>/bin` with `config/`, `script/`,
`logs/` and `heightmaps/` next to them; the servers read those relative to
their working directory.

## Source layout

| Folder | What |
|---|---|
| `src/ConnectionServer`, `LoginServer`, `ChatServer`, `PingServer`, `ZoneServer` | the servers |
| `src/NetworkManager`, `MessageLib`, `SwgProtocol` | the SWG network protocol and its messages |
| `src/DatabaseManager` | database access (MySQL Connector/C++) |
| `src/ScriptEngine` | Lua 5.1 scripting, bound with tolua++ |
| `src/Common`, `src/Utils`, `src/anh` | shared code |

## Scripting

Game content is moving from C++ into Lua scripts (`data/script`), so it can
change without rebuilding. Today the tutorial and a few world scripts are
Lua; the plan is in
[Scripting](https://github.com/swganh/mmoserver/blob/master/docs/SCRIPTING.md).
Lua must stay at version 5.1, which the bindings are written for.
