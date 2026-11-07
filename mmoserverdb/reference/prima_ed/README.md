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

Used by: migration 007 (`tools/reference/weapons_sql.py` writes it from
`weapons.csv`) and the combat rules (`data/script/combat/rules.lua`).

Not transcribed yet: the crafting ranges (pp. 391, 393; the range of each
stat a crafted weapon can have) and the armour's per-piece HAM costs
(pp. 394-395).

Read with care where the scan was faint: please report any value that
looks wrong against the page.
