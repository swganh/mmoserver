# Jump to Lightspeed in the Publish 14 client

What the expansion's content is in the client this server supports, from
`tools/client/jtl_inventory.py` run on the test client's 50 Publish 14
archives (`tools/client/publish14_archives.txt`), Nov 2026. Jump to
Lightspeed is parked on the roadmap (`docs/ROADMAP.md`, "Parked");
`docs/PRECU_AUDIT.md` has what the server would need. This page is the
size of the content, so that work can be planned when it comes.

Only file names, counts and column names are here: nothing of the
client's contents. To see a file, extract it from your own client with
`tools/client/tre.py extract` into a folder outside the repository.

## Summary

| Area | Files | Notes |
|---|---|---|
| Space zones | 16 terrain files | Tatooine, Corellia and Naboo with two zones each; Dantooine, Dathomir, Endor, Lok, Yavin 4; `space_09` and others (`space_env`, `space_halos`, `space_heavy1`, `space_light1`) to identify |
| Ship datatables | 388 | Under `datatables/space/` (chassis, components, weapons, missiles, countermeasures, turrets, asteroid fields, nebulas, space stations, formations, hyperspace) and `datatables/space_command/` (droid programs) |
| Ship object templates | 368 | `object/ship/`, 46 of them in `object/ship/player/` |
| Ship components and parts | 1,846 | Components 1,084, crafted 412, attachments 328, interior 16, customization 6 |
| Ship control devices | 52 | `object/intangible/ship/`: what sits in the datapad |
| Shipwright schematics | 320 | Weapons 93, chassis 53, boosters 28, capacitors 26, engines 26, shields 26, droid interfaces 16, reactors 16, armour 15, repair 11, reverse engineering 10 |
| Space stations, asteroids, debris | 114 | `object/static/space/` |
| Space quests | 752 quest lists, 752 task tables, 754 string files | 18 kinds: assassinate, convoy, delivery (with and without pickup), destroy, destroy duty, surprise attack, escort, escort duty, inspect, patrol, recovery, recovery duty, rescue, rescue duty, space battle, survival |
| Space mission strings | 45 | `string/en/space/`, including NPC pilots' taunts |

These files come from the general patches, not the expansion's own
`data_sku1_*` archives (which hold its art): patch 11 (the expansion's
release) supplies most of them, patches 12-14 the rest.

The pilot and shipwright skill boxes are in the general skill table
(`datatables/skill/skills.iff`), which migration 008 already matches.

## The main tables

| Table | Rows | Columns (first ones) |
|---|---|---|
| `datatables/space/ship_chassis.iff` | 313 | name, flyby_sound, hit_sound_group, wing_open_speed_factor, then each component slot with its hit weight and whether it can be targeted |
| `datatables/space/ship_components.iff` | 1,115 | name, object_template, shared_object_template, component_type, compatibility |
| `datatables/space/ship_weapon_components.iff` | 179 | name, projectile_index, range, speed, ammo_required, missile, countermeasure, mining, tractor |
| `datatables/space/missiles.iff` | 21 | |
| `datatables/space/countermeasures.iff` | 7 | |
| `datatables/space/ship_turret.iff` | 312 | chassis, weaponIndex, minYaw, maxYaw, minPitch, maxPitch |
| `datatables/space/ship_chassis_<ship>.iff` and `_tier1`-`_tier5` | 294 tables | Which components each ship's slots take. Ships: A-wing, assault ship, B-wing, civilian shuttle, speeder, taxi and transport, corvette, Decimator, Firespray, light, medium and heavy freighters, Lambda shuttle, Nebulon, TIE advanced, aggressor, bomber, fighter, interceptor and oppressor, X-wing, YKL-37R, YT-1300, Y-wing, Z-95 |

Which of these ships players could fly at Publish 14 is a question for
the guides and other sources: the client carries the data for some that
were only for NPCs, or arrived later.

## What it means for the plan

- **Most of the content is data the client already has.** Ships, their
  slots, components and weapons are tables; the server needs to load them
  (as `tools/client/` does for skills and commands), not author them.
- **What the client does not have** is the server's side: component
  stats as rolled or crafted, NPC ship loadouts and AI, spawn areas,
  mission rewards, and the combat rules (damage to components, shields,
  capacitor). Those come from the guides (The Total Experience covers
  space, with the caveat in `docs/SOURCES.md`) and other public sources
  of the time.
- **The space quests are tables too** (752 lists and their tasks): the
  steps and strings are in the client; the server runs them.
- **Next groundwork**, when space comes off the parked list: read
  `ship_chassis`, `ship_components` and one ship's chassis table in full,
  to design the ship object; then document the ship object's messages.

## Running the inventory again

```bash
python3 -I tools/client/jtl_inventory.py <folder with the .tre files> --archives tools/client/publish14_archives.txt > jtl_inventory.md
```

Run it from the top of the repository. In Windows PowerShell 5.1, `>`
writes UTF-16; that is fine to send.
