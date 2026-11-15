# Pre-CU feature audit

What the server has against what the game had at Publish 14, the last
pre-CU publish (Nov 2026). It updates the coverage table in
`docs/CODE_REVIEW_2026.md`, section 5, from a read of the code and the
database scripts and from what `docs/TEST_PLAN.md` confirms with a client.

**Tested** means confirmed with the client on a test server. **Code**
means the code or data is there but no one has played it on this server.
**Partial** means some of it works and the gaps are listed. **Stub** means
the handlers exist and do little. **Missing** means nothing is there.

## Summary

The core loops (log in, travel, survey, craft, entertain, fight creatures,
heal, trade) are in place. The gaps are in three areas:

1. **The world is empty.** Creature spawns cover the area around Bestine (4
   lairs, 92 static spawns). Every other planet has NPCs from the client's
   buildout but no wildlife, and there are no dungeons, theme parks or
   points of interest.
2. **Systems that were never built.** Factions and the Galactic Civil War,
   guilds, player cities, vendors, pets, and Jedi.
3. **Professions with commands but no effects.** Bounty hunter, smuggler,
   commando, squad leader, ranger, creature handler and bio-engineer have
   only a few handlers each.

## By system

### Accounts, galaxy and characters

| System | State | Notes |
|---|---|---|
| Login, galaxy list, character creation and deletion | Tested | Argon2id passwords, account limits (#3, #3a, #3e, #6) |
| Tutorial and skipping it | Tested | Parts 3-9 raced through (#21) |
| Character switching, reconnects, link-dead | Tested | #2, #2c |
| Character limits per account | Partial | Changed by hand in SQL (ROADMAP phase 2) |
| Admin and CSR commands | Code | `OCAdminHandlers.cpp`; not inventoried yet |

### World

| System | State | Notes |
|---|---|---|
| Planets and buildout (buildings, terminals, NPCs) | Tested on Tatooine, the tutorial and Lok | 4,701 persistent NPCs in the database |
| Travel: shuttles, starports, tickets, other planets | Tested | #5k, #5l |
| Regions: cities, badge areas, no-build zones | Tested | #16 |
| Badges | Partial | Location badges tested; others (exploration, kills, accumulation) not checked |
| Weather | Code | `Weather.cpp` |
| Creature spawns and lairs | Partial | Around Bestine only; levels, HAM and resources from the Prima tables (migration 011) |
| Creature AI: roam, aggro, assist, leash | Tested | Behaviour in C++; to move to `ai/` scripts (phase 3) |
| Dungeons (Corellian corvette, Death Watch bunker, Geonosian lab) | Missing | |
| Theme parks (Jabba's palace, Imperial and Rebel) and quest chains | Missing | 740 quest names in `quests.sql`; `QuestGiver.cpp` is a base only |
| Points of interest and NPC camps | Missing | |
| Static NPC conversations | Partial | Tutorial conversations work; trainers and others not checked |

### Combat and death

| System | State | Notes |
|---|---|---|
| HAM, to-hit, damage, weapons, postures, states | Tested | Rules in `data/script/combat/rules.lua`; `docs/COMBAT.md` |
| Specials, including area and cone attacks | Tested | Migrations 010, 012, 013 |
| Wounds, battle fatigue, bleeding | Tested | |
| Players' worn armour in combat | Missing | Needs the 2004 guide, pp. 168-169 |
| Encumbrance | Missing | |
| Dizzy, knockdown, stun, blind, intimidate | Partial | Brawler moves tested; the rest of the states not reviewed |
| Poison and disease (damage over time) | Missing | |
| Duels | Code | |
| Incapacitation, death, cloning | Tested | #24 |
| Insurance and item decay | Partial | `InsuranceTerminal.cpp` exists; decay on death not checked |
| Loot from creatures and NPCs | Partial | Loot windows work (#10, #17); 996 loot templates, never checked against the guides |

### Factions and PvP

| System | State | Notes |
|---|---|---|
| Faction points and standing | Partial | Stored per character (`character_faction`); points earned only in code paths not checked |
| Joining a faction, rank, recruiters | Missing | |
| Overt and covert, TEF, faction PvP | Missing | `CombatManager.cpp`: "both need to be overt (TODO)" |
| Faction bases, battlefields, GCW planet control | Missing | |

### Professions

The skill tree matches the client's Publish 14 table (migration 008); what
the boxes give is below.

| Profession | State | Notes |
|---|---|---|
| Marksman, brawler, and the weapon elites (pistoleer, rifleman, carbineer, fencer, swordsman, pikeman, TKA) | Partial | Specials and moves tested; elite reviews and cones still to do |
| Medic, doctor, combat medic | Partial | Healing, wounds, First Aid tested; stimpacks (#25), buffs, poison and disease to review |
| Entertainer, dancer, musician | Tested | Including healing mind and battle fatigue |
| Image designer | Code | `ImageDesignManager.cpp` (937 lines) |
| Artisan and the crafting professions | Code | Crafting, experimentation and schematics are the largest system |
| Scout | Partial | Harvesting corpses and foraging; camps and traps not checked |
| Ranger | Stub | 3 handlers (72 lines) |
| Creature handler | Missing | Taming, pets and their commands; handler file is empty |
| Bio-engineer | Stub | 53 lines |
| Bounty hunter | Stub | No bounty missions or player bounties |
| Smuggler | Stub | Feign death only; no slicing or spice |
| Commando | Stub | |
| Squad leader | Stub | 108 lines; group buffs not done |
| Politician | Missing | Needs player cities |
| Merchant | Missing | Needs vendors |
| Architect, armoursmith, weaponsmith, droid engineer, tailor, chef | Code | Through crafting; droids as pets missing |
| Jedi: unlock and Force powers | Missing | Some Force handlers exist, no unlock or Force powers system |

### Economy

| System | State | Notes |
|---|---|---|
| Resources: spawning, rotation, surveying, sampling | Tested | #5a, #5b |
| Harvesters, power, maintenance | Code | Stored functions (`sf_Harvest*`) |
| Factories and crates | Code | Stored functions (`sf_Factory*`) |
| Crafting, experimentation, schematics | Code | |
| Bank, credits, trading between players | Code | Trade window used during tests |
| Bazaar | Partial | Listing and buying tested on the `bazaar` branch (#2i-#2k); details, retrieve and cancel next |
| Player vendors | Missing | |
| Mission terminals: destroy, deliver, survey, crafting, entertainer, recon | Code | `MissionManager.cpp`; bounty, hunting and escort missions missing |

### Housing and communities

| System | State | Notes |
|---|---|---|
| Placing houses, permissions, entry and ban lists, transferring | Code | `StructureManager.cpp` |
| Furniture and decorating | Code | |
| Guild halls (placing) | Code | Placement only |
| Guilds (membership, tags, guild chat) | Missing | Nothing in the ChatServer |
| Player cities (ranks, mayors, taxes, civic buildings) | Missing | `City.cpp` is a region only |
| Groups: invites, leader, group XP, loot rules | Code | `GroupManager.cpp` |
| Mail, chat rooms, friends and ignore lists | Tested | #2f, #2g, #22 |

### Getting around

| System | State | Notes |
|---|---|---|
| Swoops and speeders | Partial | `VehicleController.cpp` calls, stores, mounts; vehicle crafting and damage not checked |
| Creature mounts | Missing | Needs pets |
| Pets and droids (calling, commands, droid modules) | Missing | |

## Jump to Lightspeed

The expansion (October 2003) is part of the pre-CU era: the Publish 14
client players have includes its space zones, ships and skills, and the
database already has the pilot and shipwright skill boxes and the space
zones in `planet`. **Nothing of space is in the server**: no ship
objects, no space zones running, no space combat.

What it would take, roughly in order:

1. **Ship objects and their messages.** The ship object type, its
   baselines and the ship movement messages (ships move by a different
   message from players). This is the main unknown: it has to come from
   the client and from public documentation, not the leaked source.
2. **Space zones.** A ZoneServer running a space scene: no terrain
   heightmap, 3D movement, a much larger scene, and the transition from a
   starport or ship terminal into space and back.
3. **Ships and components.** Chassis, components (reactor, engine,
   shields, armour, capacitor, weapons, booster, droid interface), the
   ship's datapad control device, loadouts and mass. Data comes from the
   client's ship datatables.
4. **Space combat.** Hits to components and armour, shields, capacitor
   energy, missiles and countermeasures, droid commands.
5. **Space NPCs.** Ship AI, squadrons, spawn areas, space stations.
6. **Pilot professions and space missions.** Three pilot factions,
   their trainers, the duty and destroy missions, tier quests.
7. **Shipwright.** Ship and component crafting, reverse engineering.
8. **Multi-crew ships** (the YT-1300 and others): interiors, turrets.

That is about the size of everything on the ground the project has done
so far.

### Is now the time?

Not for the expansion itself: it sits on systems the ground game does not
have yet (factions for the pilot professions, pets for droids, a fuller
crafting test), and the ground world is still mostly empty. Each step above
needs the earlier steps working with a client before the next is worth
building.

Groundwork that does not wait on the ground game, and that helps it as
well:

- **Client data inventory.** Extend `tools/client/` to list the JTL
  datatables and templates (ship chassis, components, space zones,
  pilot quests) in the Publish 14 client, so the size of the content is
  known.
- **Message research.** Document the ship object and its movement messages
  from the client, in `docs/`, with sources.
- **Space zone smoke test.** Start a ZoneServer on a space scene and see
  what breaks: heightmaps, the spatial grid's size, zone selection.

## Suggested order for the ground game

From this audit, after the bazaar pass:

1. **Fill the world**: creature spawns and lairs for every planet from the
   Prima tables and the client's spawn data, which also exercises the AI
   and combat already built.
2. **Factions and the GCW**: joining, rank, overt and covert, TEF,
   faction kills. Needed by the PvP, the pilot professions and the theme
   parks.
3. **Finish the combat professions**: worn armour, the states, poison and
   disease, the elites, then the stub professions (squad leader, commando,
   bounty hunter with bounty missions, smuggler).
4. **Pets**: creature handler, droids, creature mounts.
5. **Communities**: guilds, vendors and merchants, player cities and
   politicians.
6. **Content**: quests, theme parks, points of interest, dungeons (phase
   4, on the script API from phase 3).
7. **Jedi**, last, as it was in the game.

Alongside: a client test pass of what is marked Code above (crafting,
harvesters, factories, housing, missions, groups), which may find working
systems or bugs before new work starts.
