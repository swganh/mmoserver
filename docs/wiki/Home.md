MMOServer is a server emulator for Star Wars Galaxies, aiming at the
Pre-Combat Upgrade (pre-CU) experience. It was originally developed by the
SWG:ANH team and is being revived: it builds on current Linux distributions,
installs with a few commands, and has been played end to end with a real
client (the whole tutorial, Tatooine, surveying, looting, travel between
planets).

## Start here

| I want to... | Read |
|---|---|
| Run a server on Linux | [[Linux Quick Start]], then [[Running the Servers on Linux]] |
| Understand how the pieces fit together | [[Architecture]] |
| Set up or manage the database | [[Database]] |
| Connect a game client | [[Game Client]] |
| Fix something that does not work | [[Troubleshooting on Linux]] |
| Develop on Windows | [[Windows Development]] |
| Help with the code | [[Contributing]] |

## Platforms

- **Linux** is the platform for running servers: Ubuntu 24.04 and 26.04 LTS,
  tested in CI and with a client.
- **Windows** is for development: building and debugging the servers in
  Visual Studio; see [[Windows Development]].
- Everything else (the database, the game client, the architecture) is the
  same on both, under **Shared** in the sidebar.

## Project documents

Reference material lives in the repository, next to the code it describes:

- [Roadmap](https://github.com/swganh/mmoserver/blob/master/docs/ROADMAP.md):
  where the project stands and where it is going
- [Known issues](https://github.com/swganh/mmoserver/blob/master/docs/KNOWN_ISSUES.md):
  open bugs and what has been fixed
- [Test plan](https://github.com/swganh/mmoserver/blob/master/docs/TEST_PLAN.md):
  what has been confirmed with a client
- [Scripting](https://github.com/swganh/mmoserver/blob/master/docs/SCRIPTING.md):
  the plan for moving content into Lua scripts

The wiki has the guides; the repository has the reference. These pages are
kept in the repository too (`docs/wiki`), so changes to them can be reviewed
like code.
