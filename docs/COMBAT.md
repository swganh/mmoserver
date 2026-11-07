# Combat: what the server does today

A review of the combat code as it stands (Nov 2026), from reading it and
from fights on the VM: what is implemented, the actual maths, what is
missing, and what should move to scripts or data. The original pre-CU rules
come from the Prima guides (`docs/SOURCES.md`); this page describes the
code, not the original.

## One fight, step by step

1. **The attack command.** The client sends `attack` (or a special) with a
   target. It goes into the player's command queue
   (`ObjectController::enqueueCommandMessage`). Combat commands
   (`command_table.addtocombatqueue = 1`) run one at a time, each followed
   by its cooldown, `command_table.defaulttime`: **2000 ms for `attack`**,
   whatever the weapon. Non-combat commands are put ahead of combat ones.
2. **Auto-attack.** After the first attack the server keeps attacking the
   same target: when the queue is empty and the cooldown has passed, it
   queues an internal `attack` (`_processCommandQueue`).
3. **Checks** (`CombatManager::handleAttack`, `_verifyCombatState`): a
   weapon in the left hand, the target within range (6 m, pistols 35 m,
   carbines 50 m, rifles 64 m; hard-coded), the target attackable, not
   incapacitated or dead. Both sides go into the Combat state and onto each
   other's defender lists.
4. **Hit or not** (`_hitCheck`): a fixed roll, `rand() % 12`:
   0 or 5-11 hit (8 in 12, **67%**), 1 miss, 2 block, 3 dodge, 4 counter
   (1 in 12 each). Only a hit does damage. No accuracy, defence, posture,
   stats, skills or range modifiers.
5. **Damage** (`_executeAttack`): uniform in [weapon min, weapon max),
   times the command's `damage_multiplier` if it has one. Mitigation
   (armour, resistances) is an empty TODO.
6. **Which pool.** If the command names no pool (the default attack names
   none), health, action or mind at random, **a third each**. Specials can
   name one (`health_hit_chance` etc., used as on/off, not as a chance).
7. **States.** If the command has a knockdown, dizzy, blind, stun,
   intimidate or posture-down chance, the state is **always** applied: the
   "chance" is never rolled.
8. **Incapacitation and death.** When any pool reaches 0, the creature is
   incapacitated (`Ham::updatePropertyValue` → `CreatureObject::incap`);
   creatures die straight away, players are incapacitated (death after a
   configured number of incaps in a row, or a death blow in a duel).
9. **Regeneration** (`Ham::updateRegenRates`, every second): a player gets
   `constitution / 100` health per second (stamina, willpower for action,
   mind; divisors in the config, x1.25 crouched, x1.75 sitting, x0.75
   knocked down). A creature gets **a tenth of that, plus 1**.
10. **XP** (`NPCObject::updateAttackersXp`): the creature's `creature_xp`
    goes to the player or group that did the most damage, split equally
    between the weapon types they used; combat XP is a fraction of it.
    Loot rights go to the same player or group.

Creatures attack the same way (`NpcManager::handleAttack`): their own damage
range (`creature_damage_min/max`), every `creature_attack` seconds, the same
67% hit roll and the same random pool. The to-hit figure the Prima tables
give per creature (e.g. 28% for a womp rat) is not used.

## Why a womp rat took 7 minutes

Novice artisan, CDEF pistol (17-35 damage), against a womp rat with 900 in
each pool (Prima p. 184):

| | per second |
|---|---|
| attacks (one per 2 s) | 0.5 |
| hits (67%) | 0.33 |
| damage (average 25.5) | 8.5 |
| on any one pool (a third) | 2.8 |
| creature regen per pool | -1 |
| **net loss per pool** | **about 1.8** |

900 / 1.8 = about 490 s for a pool, a little less for the first of the
three: about 7-8 minutes, as seen. The creature's 90-110 damage, spread the
same way over the player's larger, faster-regenerating pools, roughly
cancels out, so neither side wins quickly.

What made it slow, in order of effect: damage spread over three pools
instead of one; no skill or stat modifiers at all; creature regeneration
during the fight; the flat 2 s cadence (the pistol's own speed, 3.2 s, is
not used, so in that respect players attack faster than they should).

## What is implemented

- The command queue with cooldowns, auto-attack, the Combat and Peace
  states, defender lists, `/peace`.
- Default attacks with any weapon; damage from the weapon's min/max.
- HAM pools, incapacitation, death, clone selection, regeneration with
  posture modifiers.
- Duels between players; death blows in a duel.
- Creature AI: idle, roaming, aggro within `creature_aggro_range`,
  stalking, attacking, assisting a lair; `State_Dead` (since the combat
  branch: a dead creature stops attacking).
- XP by most damage, per weapon type, group split; loot rights.
- Combat spam, fly text (miss, block, dodge, counter), animations.
- Ability costs (`healthcost` etc.) are charged for commands that have them
  (`ObjectController.cpp`); 71 commands have costs.

## What is not

- **The to-hit formula:** no accuracy (weapon, skill mods), defence
  (dodge, block, counterattack skill mods), posture, range modifiers
  (the weapon's point blank / ideal / max range), target's level, or the
  creature table's to-hit.
- **Damage modifiers:** no skill or stat bonuses, armour, resistances or
  vulnerabilities (the attributes exist on creatures and items, unused),
  no wounds (weapons have a wound chance) or battle fatigue.
- **Where damage lands:** pre-CU attacks hit a body location which decided
  the pool; here, a random pool for default attacks.
- **Attack speed:** weapon speed and speed skill mods are ignored; every
  `attack` is 2 s.
- **Special attacks:** the command table has `attack`, `berserk1`,
  `intimidate1`, `warcry1` and the like, but no weapon specials
  (headshot, bodyshot, legshot...) for the professions.
- **State chances** are not rolled; states have no durations or
  defences; knockdown/dizzy interplay is missing.
- **Creatures:** no special moves, no fleeing, no healing; creature
  regeneration is a made-up rule; aggression ignores the player's level.
- **Combat XP** rates and weapon XP split are placeholders ("a complete
  made up rule" in the code).
- **"Too many commands queued"** (KNOWN_ISSUES #5h): still seen with
  attacks queued. The server now logs what fills the queue when it
  overflows (`Command queue full for ...` in the zone log).

## Where each part should live

The aim (`docs/SCRIPTING.md`) is C++ for the engine, data for numbers,
scripts for behaviour:

| Part | Where | Why |
|---|---|---|
| The command queue, cooldowns, auto-attack, states, HAM, regen ticks, incap/death | C++ | the engine; runs every tick for everyone |
| The to-hit and damage formulas | C++, with their constants in data | run on every attack; tuning without rebuilding |
| Weapon stats, creature stats, skill mods, armour | data (items, `non_persistent_npc_attributes`, skill tables) | already data; needs the right values (Prima) |
| Special attacks: costs, multipliers, pools, state chances, animations | data (`command_table`) | the server already reads all of these columns |
| Creature behaviour: aggro, fleeing, special moves, calling for help, the tutorial bandit's grenade | scripts (`ai/`), on events | content, varies per creature |
| Combat events for scripts: attack, hit, incap, death, XP | C++ raises them (`creature_died` exists) | quests and AI react to them |
| XP and loot rules | C++ with data tables | small and per kill |

## Order of work

1. Find what overflows the command queue (the new log line, next fight).
2. Read the original rules: the Prima combat chapter (2004 QRG pp. 40-48,
   and the Empire Divided guide's), and write the formulas down here with
   page citations before changing code.
3. To-hit and damage from those rules, with the constants in data; weapon
   speed; damage to the pool the attack hits.
4. Creature regeneration and to-hit from the creature tables.
5. Specials as data, profession by profession (marksman and brawler first:
   the starting professions).
6. Creature behaviour as `ai/` scripts.
