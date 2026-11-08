# Combat: what the server does today

A review of the combat code as it stands (Nov 2026), from reading it and
from fights on the VM: what is implemented, the actual maths, what is
missing, and what should move to scripts or data. The maths of an attack
is now in a script, `data/script/combat/rules.lua` ("The combat rules"
below). The original pre-CU rules
come from the Prima guides (`docs/SOURCES.md`); this page describes the
code, not the original.

## One fight, step by step

1. **The attack command.** The client sends `attack` (or a special) with a
   target. It goes into the player's command queue
   (`ObjectController::enqueueCommandMessage`). Combat commands
   (`command_table.addtocombatqueue = 1`) run one at a time, each followed
   by its cooldown: for attacks, the time the combat rules give (the
   weapon's attack speed); for other combat commands,
   `command_table.defaulttime`. Non-combat commands are put ahead of
   combat ones.
2. **Auto-attack.** After the first attack the server keeps attacking the
   same target: when the queue is empty and the cooldown has passed, it
   queues an internal `attack` (`_processCommandQueue`).
3. **Checks** (`CombatManager::handleAttack`, `_verifyCombatState`): a
   weapon in the left hand, the target within the weapon's max range
   (`rules.inRange`), the target attackable, not incapacitated or dead.
   Both sides go into the Combat state and onto each other's defender
   lists.
4. **Hit or not, damage, pool, attack time:** the combat rules
   (`rules.attack`), below. A miss shows "miss"; block, dodge and counter
   are not rolled (no rule for them yet).
5. **Damage** goes to the pool the rules chose (`CombatManager::applyDamage`).
6. **Specials** pass their `damage_multiplier`, `delay_multiplier` and pool
   (`health_hit_chance` etc., used as on/off, not as a chance) to the
   rules.
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

Creatures attack the same way (`NpcManager::handleAttack`): every
`creature_attack` seconds, through the same rules, with their own damage
range (`creature_damage_min/max`) and no weapon range modifiers. The to-hit
figure the Prima tables give per creature (e.g. 28% for a womp rat) is not
in the database yet; the rules take it as `to_hit` when it is.

## The combat rules

`data/script/combat/rules.lua`, loaded by every zone at start (its own Lua
state, called directly on each attack; `src/ScriptEngine/CombatRules.*`).
If the script is missing or fails, attacks fall back to the old rules (two
hits in three, the weapon's damage, `defaulttime`). Unit tests:
`src/ScriptEngine/combat_rules_unittest.cc`.

What it works out, from the guide where it gives the rule and with
**reconstructed** numbers (ours, marked in the script, the ones to tune)
where it does not:

| | Rule | Source |
|---|---|---|
| Accuracy modifier | the weapon's range curve (point blank at 0 m, ideal at its ideal range, max at its max range, straight lines between) + the weapon group's accuracy skill mods + postures - the defender's ranged or melee defence | p. 66, p. 73, p. 145 |
| Postures | attacker kneeling +10, prone +20; a kneeling/prone target -10/-20 against ranged attacks, +10/+20 against melee | p. 73; sizes reconstructed |
| Chance to hit | 50% + 1% per point of accuracy modifier, 5-95% | reconstructed |
| Creatures | their own to-hit replaces the 50% when known | creature tables |
| Damage | uniform in the weapon's range x the special's multiplier x armour rating (+25% per level the weapon is above the armour, -50% per level below) x (1 - armour effectiveness for the damage type) | p. 388, pp. 394-395 |
| Pool | the special's, else health 50%, action 30%, mind 20% | reconstructed |
| Attack time | the weapon's attack speed x (1 - 1% per point of speed mod) x the special's delay multiplier, at least 1 s | p. 389, p. 145; the 1% reconstructed |
| Distance | centre to centre less 2 m for the bodies, as the client measures | target window |
| Range | the weapon's max range + 2 m for the bodies | p. 390, target window |

Weapons carry their ranges in metres as internal attributes
(`combat_range_mid`, `combat_range_max`; migration 007, from
`mmoserverdb/reference/prima_ed/weapons.csv`); weapons without them use
their group's typical ranges. Players' worn armour is not read yet (no
armour on the defender's side); creatures' `armor_rating` and
`cat_armor_effectiveness.*` are.

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

With the rules and the noob CDEF (50-80, 4 s; new characters since
migration 007), the same novice artisan at 10 m: modifier +17, 67% to
hit, 10.8 damage a second, half of it on health: 900 / (5.4 - 1) = about
3.5 minutes for the womp rat, a level 10 creature; about 35 s for a lesser
desert womp rat (155). A novice marksman (+10 accuracy, +5 speed) at 15 m:
90% to hit, about 2 minutes for the womp rat.

What made it slow before the rules, in order of effect: damage spread over three pools
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

## The target window's accuracy

The client works out the accuracy modifier it shows; the server does not
send it. It needs the weapon's range data, which the server never sent
(weapons go to the client as plain tangible objects), so the window showed
a number only next to the target. The SWGANH wiki lists an object
controller message for it, WeaponRanges (0x140): after the header, a LONG
and five INTs, otherwise undocumented ("25%", Jan 2008). The server
sends it to the player on every target change: the weapon's id, then
`rules.clientRanges(weapon)`: ideal range, max range (floats), point blank,
ideal and max modifiers (integers), worked out below. If
`rules.clientRanges` returns nothing, no message is sent. The script also
picks each field's type (`"i"` int32, `"f"` float). The probes below were
sent from a probe file the script read on each target change (removed
once the layout was found).

Tries, marksman with the noob CDEF against a womp rat:

| Layout | Target window |
|---|---|
| `iiiii` | no change: -5 on top of the target armed, -15 unarmed |
| `fffff` -10, 15, 30, 64, -90 | the client takes it: +50 flat from about 5 m to 0 m, "--" (out of range) from about 5 m to 66 m |
| `fffff` -21, 12, 33, 24, -45 (probe, every weapon) | "--" everywhere |

From here one field at a time, from try 2, with the probe file
(`script/combat/client_ranges_probe.txt`, read on every target change).
The window read +50 in each, with "--" beyond the distance given:

| Probe | Fields | Shown up to |
|---|---|---|
| A | -10 15 30 64 -90 | 5 m |
| B | -10 **25** 30 64 -90 | 10 m |
| C | **0** 15 30 64 -90 | 9 m |
| D | -10 15 30 **20** -90 | 4 m |
| E | -10 15 **0** 64 -90 | 3 m |
| F | -10 15 30 64 **0** | 16 m |

Every field moves the edge, so "--" looks like the accuracy falling below
a floor rather than a max range, and +50 a cap on the display. The best
straight-line fit (point blank = 4th field, ideal range = 2nd, ideal
modifier = 5th, floor about 0) matches A, B, D and F but not C and E, so
it is not the answer.

Small values (G baseline `0 15 0 20 0`, one field changed in each):

| Probe | Changed | 1 m | 2 m to the edge | Edge |
|---|---|---|---|---|
| G | none | -5 (-25 while moving) | +50 | 17-18 m |
| H | 1st 0 → 10 | -5 | +50 | 18 m |
| I | 2nd 15 → 25 | -5 | +50 | 26-27 m |
| J | 3rd 0 → 10 | +50 | +50 | 18 m |
| K | 4th 20 → 30 | -5 | +50 | 18 m |
| L | 5th 0 → 10 | -5 | +50 | 18 m |

Found: the 2nd field is the max range (the edge is about 2 m past it,
probably the bodies' radii). The window adds the client's own modifiers
(moving: -20). Not yet: the other four; the number between 2 m and the
edge is +50 whatever they are, so the display seems capped at +50 with
the client's own base putting it over.

Uniform values, max range 20: `-20 20 -20 -20 -20`, `-40 ...`, `-60 ...`
showed "--" even at 1 m; `20 20 20 20 20` showed +50 from 1 m to about
21 m. Nothing in between: the client may read the modifiers as fractions
(0.30 for +30), so 20 is far over the cap and -20 far under the floor.

Fractions, max range 20: 0.1 and 0.3 everywhere showed +50, -0.1 "--",
all 0 showed -5 at every distance (-35 walking). So not fractions: the
modifier fields are integers, and a float's bits read as an integer are
huge, positive or negative with the float's sign. The distances behaved
as floats. Layout found: **ideal range (float), max range (float), point
blank modifier, ideal modifier, max modifier (integers)**, "ffiii". It
fits every probe from G on (A-F had a negative ideal range). The client
adds its own base (-5 for this marksman standing) and modifiers (moving).

Checked with the noob CDEF's own values (`ffiii 15 64 -10 30 -90`),
marksman standing, womp rat: -15 at 2 m, +3 at 8 m, +23 at 16 m, -31 at
40 m, -50 at 64 m ("--" beyond). That is the guide's curve, 2 m further
out (the client measures between the bodies, not the centres), plus the
client's -5, shown between -50 and +50. The rules now measure the same
way (`RANGE_ALLOWANCE`), so the chance rolled follows the one shown. The -5
is the client's own: a brawler (no pistol accuracy mod) read the same as
the marksman (+10) with the noob CDEF (-15, +5, +23, -31, -50 at 1, 8,
16, 40, 64 m), so the window leaves skill mods out. An artisan read -5,
+11, +30, -27, -50, which fits the plain CDEF (0, +35, -80). The ranges
are sent on every target change and on every equip or unequip (at first
only on target change, so a weapon switch showed the old weapon's curve
until the next target).

Re-test with new Rodian characters, noob CDEF: a brawler read -15, +6,
+24, -33, -50 and an artisan -15, +4, +25, -31, -50 at 1, 8, 16, 40,
64 m: the same curve for both. Unarmed (the server sends ideal 3 m, max
5 m, all modifiers 0): -15 up to about 6 m, "--" beyond. So the client
takes the reach, but its own base is -15 unarmed against -5 with the
pistol; where the 10 comes from (weapon type, the target's defence
against melee) is not known.

## What is not

- **To-hit:** no block, dodge or counterattack (the skill mods exist),
  certification penalty, moving penalty, states (aiming, blinded...),
  the "no point blank attacks while prone" rule; creature to-hits are not
  in the database yet.
- **Damage modifiers:** no skill or stat bonuses, players' worn armour,
  vulnerabilities, wounds (weapons have a wound chance) or battle fatigue.
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
| The to-hit and damage formulas | script (`combat/rules.lua`) | run on every attack; communities rebalance without rebuilding |
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
(damage min 17-32, max 35-65: p. 393) and characters got one at the
bottom of them (17-35, as seen); the noob CDEF (2755) matched p. 392 but
for the zero and max range mods (0 and -80 against -10 and -90). Migration
007 fixes those, starts new characters with the noob CDEF, and adds every
weapon's ideal and max range in metres. The other weapons already matched
the guide but for empty values (cleaver, katana, sword 02, Jawa ion rifle)
and the point blank modifier of most pistols (0 for -5 to -20).

### Against the code

| Rule | Code today |
|---|---|
| Accuracy roll from range mods, posture, certification, states | range mods, skill mods, postures (rules.lua); not certification or states |
| Attack time from the weapon's attack speed | the weapon's speed and speed mods (rules.lua) |
| Armour rating and effectiveness | creatures' (rules.lua); not players' worn armour |
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
3. To-hit and damage from those rules: done, in `combat/rules.lua`; the
   weapons' ranges and the noob CDEF: migration 007. Next: tune the
   reconstructed numbers in play.
4. Creature regeneration and to-hit from the creature tables.
5. Specials as data, profession by profession (marksman and brawler first:
   the starting professions).
6. Creature behaviour as `ai/` scripts.
