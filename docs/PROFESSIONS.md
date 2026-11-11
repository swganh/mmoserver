# Professions: the database against the client and the guide

**Baseline: the client's own skill table, Publish 14**
(`datatables/skill/skills.iff`), which the client builds the skill tree,
mods and commands from. `tools/client/skills_compare.py` compares it with
the database; migration 008 made all 1,068 skills match (16 boxes
differed). Run the tool again after any skill change.

Below, the first review: the marksman and brawler boxes against Prima's
*An Empire Divided* (2003, the launch guide; `docs/SOURCES.md`), read
from the page images, before the client's table was at hand. Where the
guide and the client differ, the client (later pre-CU) wins: unarmed
damage, Center of Being, the rifle concealment chance and the two-handed
certifications' boxes are Publish 14's. Mods named `private_*` are
internal and not in the guide.

## Marksman (pp. 145-148)

Matches the guide: every box's XP cost and skill points, all accuracy and
speed mods, every special and every certification, but:

| Box | Guide | Database |
|---|---|---|
| Pistol Specialist (`pistol_04`) | Ranged Defense +2 | missing |
| Expert Ranged Support (`support_03`) | Ranged Defense +2 | missing |
| Rifle boxes 1-4 | — | `rifle_concealment_chance` +5 each, not in the guide |
| Intermediate Carbine (`carbine_01`) | DH17 Short Carbine and DH17 Snubnose Carbine certifications | snubnose only (the two may be one weapon) |
| Master Marksman | not in the pages at hand | — |

"General Ranged Aiming" in the guide is `aim`, "Alertness" is `alert`.

## Brawler (pp. 127-131)

| Box | Guide | Database |
|---|---|---|
| Novice | Unarmed Damage +10 | 15 |
| Novice, every box | — | Center of Being (command and `center_of_being_*` mods), not in the guide |
| Advanced, Expert, Master Unarmed (`unarmed_02`-`04`) | Unarmed Damage +5 each | 10 each |
| Intermediate Polearm (`polearm_01`) | Polearm Speed +5 | **Pistol Speed +5** |
| Expert, Master Two-Hand (`2handmelee_03`, `04`) | Two-handed Melee Toughness +10 | **Polearm Toughness +10** |
| Expert, Master Polearm (`polearm_03`, `04`) | Polearm Toughness +4 | **Two-handed Melee Toughness +4** |
| Advanced Two-Hand (`2handmelee_02`) | Two-handed Cleaver certification | Katana |
| Master Two-Hand (`2handmelee_04`) | Two-handed Curved Sword and Scythe certifications | Cleaver |
| Master Brawler | Two-handed Melee Accuracy +5, Speed +5; Taunt +30; One-handed Melee Toughness +5 | missing |

The client confirms the polearm speed, the swapped toughness rows and the
Master Brawler's missing mods as mistakes (fixed by migration 008). The
rest are Publish 14 as the database has it.

## Commands

The client's command table, Publish 14
(`datatables/command/command_table.iff`, 769 commands), against
`command_table` (`tools/client/commands_compare.py`): the client times its
command queue by each command's default time, and 310 differed (1.5 s for
nearly every special against 2 s here). Migration 009 makes the times and
the combat queue use match. The client's table has no damage, cost or
to-hit figures: those were the server's. Not yet compared: the posture and
state masks (which postures and states allow a command) and the maximum
range (the client's 0 means the weapon's range).

## Specials

The 134 moves of the guide's special moves table (pp. 76-83) are in
`command_table` from migration 010; `docs/COMBAT.md`, "Specials", has how
the server uses them.
