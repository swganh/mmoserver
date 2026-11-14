# Reference data: An Empire Divided

Facts from Prima's Official Strategy Guide, *Star Wars Galaxies: An Empire
Divided* (2003, the launch guide; `docs/SOURCES.md`), appendix, transcribed
from the page images; `page` is the printed page (PDF page = page + 3).
Reference data: migrations take their values from here and cite the page.

- `weapons.csv` (pp. 390, 392; Mind cost pp. 391, 393): looted/vendor
  weapons. `zero_range_mod`, `mid_range_mod`, `max_range_mod`: accuracy
  modifiers at point blank, at `mid_range` (the ideal range) and at
  `max_range`, in metres. `attack_speed`: seconds between attacks.
  Health/action/mind costs: multiplied by a special move's modifiers;
  default attacks cost nothing (p. 389). Damage/minute = 60 / speed x
  average damage (p. 389).
- `armor.csv` (p. 394): generic effectiveness (% of damage absorbed) and
  per damage type: a number replaces the generic value, `0` means no
  protection against that type, `+` means the generic value (p. 395).

- `specials.csv` (pp. 76-83, each row across a page pair): the special
  moves table, 134 moves. `time_mod`, `damage_mod` and the costs multiply
  the weapon's attack speed, damage and HAM costs; `to_hit_mod` adds to
  the accuracy; the end postures and effects (with one chance for all of a
  move's effects, durations in seconds) apply on a hit. `area_length`
  "weapon" means the weapon's range. ActionShot2's name is missing from the
  page; it follows ActionShot1 with the same description. Polearm Sweep2's
  to-hit is printed 15 where the other polearm moves have 10; kept as
  printed.

- `creatures.csv` (creature tables, pp. 396-483, each row across a page
  pair: `page` the names and stats, `facing` the to-hit, XP and flags):
  only the creatures the server spawns that the 2004 guide's table leaves
  out (it stops at level 11; `../prima_2004/`). HAM is per pool here,
  one range for the three in 2004.

Used by: migrations 007 (`tools/reference/weapons_sql.py`, from
`weapons.csv`) and 010 (`tools/reference/specials_sql.py`, from
`specials.csv`), 011 (`tools/reference/creatures_sql.py`, with
`../prima_2004/creatures.csv`) and the combat rules (`data/script/combat/rules.lua`).

Not transcribed yet: the crafting ranges (pp. 391, 393; the range of each
stat a crafted weapon can have) and the armour's per-piece HAM costs
(pp. 394-395).

Read with care where the scan was faint: please report any value that
looks wrong against the page.
