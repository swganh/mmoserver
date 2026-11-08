# Professions: the database against the guide

The skill boxes in the database (`skills`, `skills_skillmods`,
`skills_skillcommands`) checked against Prima's *An Empire Divided* (2003,
the launch guide; `docs/SOURCES.md`), profession chapters, read from the
page images. Mods named `private_*` are internal and not in the guide.

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

The polearm and two-hand toughness rows look swapped, and the polearm
speed is a typing slip. The others (unarmed damage, Center of Being, the
rifle concealment chance, the certifications' boxes) may be later changes
the database took from another publish rather than mistakes.

## Specials

The marksman's specials are all in `command_table` with damage and delay
multipliers and pools; their HAM costs are 0 (the guide: cost modifiers x
the weapon's HAM costs, p. 74) and no accuracy bonus is set. The special
move tables (pp. 74-83: time, damage, cost and to-hit modifiers, end
postures, effects with chances and durations) are the source for them.
