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

## The original rules (An Empire Divided, 2003)

From Prima's *An Empire Divided*, chapter 5, "Combat" (pp. 65-86; PDF page
= guide page + 3), the launch-era guide (`docs/SOURCES.md`). The guide
states the rules in words; where it gives no numbers, none are invented
here.

**Accuracy.** Each attack makes an accuracy roll; the target window shows
your accuracy modifier against it, the chance to hit, "the higher the
number, the better" (p. 73). The modifier depends on:
- the distance to the target, through the weapon's three range attack
  mods: point blank, ideal (at the weapon's ideal range) and max (at its
  maximum range). The guide's laser rifle: -30, +10 at 52 m, -60 at 64 m
  (p. 66). The CDEF pistol, as the client shows it: -10, +35, -80;
- certification: an uncertified weapon gives an accuracy penalty (p. 67);
- your posture and the target's posture (below), moving (running and
  firing a pistol is penalised, p. 67), and states (p. 84);
- melee weapons such as the survival knife: "+30 chance to hit across the
  board" (p. 66).

**Postures** (p. 73). Standing: no modifier; spotted sooner. Kneeling: a
bonus to your accuracy, harder to spot, ranged attackers get a penalty
against you, melee attackers a bonus. Prone: a significant bonus to your
accuracy and a significant penalty to those firing at you, melee attackers
a huge advantage; no attacking at point blank range while prone.
A ranged weapon also gives a defensive penalty against melee attackers
(p. 70, p. 73).

**Damage** goes to one or more of health, action and mind; fly text is red
for health, green for action, blue for mind, mixed colours for two pools,
white for all three (p. 85). Some weapons cause bleeding on a hit, which
drains pools over time (p. 85); every move that wounds also causes
bleeding (p. 75).

**Wounds** lower a pool's maximum and do not regenerate: medics, combat
medics and doctors heal health and action wounds, entertainers mind wounds.
**Battle fatigue** builds up in combat; it does not affect fighting but
hampers the curing of wounds; entertainers heal it (p. 86).

**Incapacitation:** any pool at 0. No moving, attacking, talking or
equipping; a timer shows when you get up; a stimpack revives (p. 85).
**Death:** a deathblow while incapacitated, or three incapacitations within
any 10 minutes (p. 85). Killers deliver deathblows, non-killers only
incapacitate (p. 73).

**Con colours** (green, blue, white, yellow, red) are relative to your
skill with the equipped weapon and change with it (p. 72). Pack creatures
assist each other (p. 73); creatures show "?" (alerted), "!" (warning) and
a fear symbol (may flee) before engaging (p. 71).

**Special moves** (pp. 74-83), a table for every weapon type (1H and 2H
melee, polearm, unarmed, pistol, carbine, rifle, "ranged"):
- time = time modifier x the weapon's attack speed;
- damage = damage modifier x the weapon's min/max damage;
- HAM costs = cost modifier x the weapon's HAM costs (the CDEF pistol: 7
  health, 15 action, 7 mind);
- a to-hit modifier (e.g. +25 for 1H melee moves, +50 for pistol moves);
- the attacker's and defender's end posture (up, down, knocked down);
- up to three effects (stunned, blinded, dizzy, ...), each with its own
  chance on a successful hit and its own duration in seconds.

The marksman's starting moves: Point-Blank Single 1 and Point-Blank
Area 1 cancel the point-blank accuracy penalty, Overcharge Shot 1 adds
damage at the cost of weapon condition (p. 69). The table's text layer is
garbled by OCR; it will be transcribed from the page images when the
specials are built.

**States** (p. 84): aggressive (better counterattack, harder to hit),
aiming (bonus to hit on the next attack), alert (counters surprise
attacks), berserk (to hit, speed and damage up, defence down), blinded
(offence and defence down), dizzy (may fall when changing posture),
evading (chance to dodge), immobilized, in combat, incapacitated,
intimidated (easier to hit), rallied (to hit and defence up), stunned
(defence down), taking cover (defence up), tumbling (defence while
changing posture).

**The 2004 guide** (pp. 40-48) repeats this chapter almost word for word,
with two differences: an uncertified weapon gives "severe damage
penalties" (p. 41; the 2003 guide says accuracy penalties, p. 67), and on
the radar red dots are aggressive creatures, yellow dots peaceful ones
(p. 43). No formulas either.

**Skill mods and base attributes.** The profession chapters list each
skill box's mods and each species' starting attributes (Marksman,
*An Empire Divided* pp. 140-148: Novice Marksman +10 accuracy and +5 speed
for pistol, carbine and rifle, p. 145; Brawler pp. 123-131). The database
already has them (`skills_skillmods`): Novice Marksman and Novice Brawler
match the guide for accuracy and speed (the Brawler's unarmed damage is
unclear in the scan). A Novice Artisan has no combat mods. The combat code
reads none of them, nor the `private_<weapon>_combat_difficulty` mods
(100 per novice combat box) that look like the basis of the con colour.

**Not in this chapter:** the numbers behind accuracy (base chance, the
size of the posture and state modifiers, how skill mods add in),
regeneration rates, XP. Neither guide gives them. What remains: the
client's own target window, which shows the accuracy modifier the client
works out, measured against known inputs (range, postures, weapons, skill
mods).

**Weapons and armour** (appendix, pp. 388-395; transcribed in
`mmoserverdb/reference/prima_ed/`):
- every weapon has damage, attack speed (seconds between attacks), an
  accuracy modifier at point blank, at its mid (ideal) range and at its
  max range, a wound chance and HAM costs for specials. The CDEF pistol:
  25-50, 3.5 s, 0 / +50 at 15 m / -80 at 64 m; the "noob" CDEF new
  characters get: 50-80, 4 s, -10 / +30 at 15 m / -90 at 64 m (p. 392);
- **armour rating** (p. 388): weapons and armour are rated 0 (none) to 3
  (heavy); player-crafted armour is 1, creatures 0-3. A weapon rated
  higher than the armour does 25% more damage per level (100 becomes 156
  for two levels); rated lower, 50% less per level (100 becomes 25 for
  two levels);
- **armour effectiveness** (pp. 394-395): the percentage of damage
  absorbed, generic or per damage type: a number for that type replaces
  the generic value, 0 means no protection against it, "+" the generic
  value;
- default attacks cost no HAM; a special costs its modifiers times the
  weapon's costs (p. 389).

In the database: the plain CDEF (item type 2326) holds the crafting ranges
(damage min 17-32, max 35-65: p. 393) and a character gets one at the
bottom of them (17-35, as seen); the noob CDEF (2755) matches p. 392 but
for the zero and max range mods (0 and -80 against -10 and -90). No weapon
stores its attack speed or its mid range in metres.

### Against the code

| Rule | Code today |
|---|---|
| Accuracy roll from range mods, posture, certification, states | fixed 67% |
| Attack time from the weapon's attack speed | 2 s for every weapon |
| Specials: time, damage, cost and to-hit from the weapon | 4 combat specials in `command_table`; costs fixed, not from the weapon |
| Effects rolled per effect, with durations | always applied, no duration |
| Bleeding, wounds, battle fatigue | not in combat |
| Three incaps in 10 minutes means death | `IncapResetTime` is configurable; to check against the 10 minutes |
| Prone: no attacking at point blank | no |
| Con colour by weapon skill | client side; the server sends levels |

## Order of work

1. Find what overflows the command queue (the new log line, next fight).
2. Read the original rules: done (both guides' combat chapters, the
   Marksman and Brawler chapters). The numbers behind accuracy are in
   neither guide: measure the client's accuracy display instead.
3. To-hit and damage from those rules, with the constants in data; weapon
   speed; damage to the pool the attack hits.
4. Creature regeneration and to-hit from the creature tables.
5. Specials as data, profession by profession (marksman and brawler first:
   the starting professions).
6. Creature behaviour as `ai/` scripts.
