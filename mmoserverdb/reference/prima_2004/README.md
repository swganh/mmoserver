# Reference data: the 2004 guide

Facts from the Prima Official Game Guide (2004; `docs/SOURCES.md`), the
creature section's tables (printed pp. 172-207), in our own columns.
`page` is the printed page with the names and stats, `facing` the page
opposite with the rest of the row.

- `creatures.csv`: 683 rows: creatures (265, levels 1-11), androids and
  droids (33) and NPCs (385). Written by
  `tools/reference/prima2004_tables.py` from the PDF, which pairs the two
  pages of each row by their position on the page; rows were checked
  against the page images (pp. 172-173, 184-185, 192). Columns:
  - `ham_min`, `ham_max`: one range for health, action and mind;
  - `to_hit`: base chance to hit, in percent;
  - `damage_min`, `damage_max`: base damage; `xp`: combat XP for a kill
    (the guide calls it a guideline);
  - `meat`, `hide`, `bone` and their `_qty`: what a scout harvests; an
    empty quantity is printed empty (`herbivore ()`); `milk`: `milk` or
    `milk (wild)`;
  - `pvp_faction`, `weapons`: NPCs only;
  - `notes`: as printed: Aggro (attacks on sight), Killer (deathblows an
    incapacitated player instead of leaving them), Stalker (follows and
    attacks later), Pack (assists its social group), Herd (moves in a
    group, does not assist unless also Pack), Tamable, Can't be harmed,
    Offers missions, Healer, and special attacks.

Printed oddities kept as printed: "Spider Nightsider (PvP)", "Plasma
Theif", "Vetran explorer"; Barada's location is blank.

Used by: migration 011 (`tools/reference/creatures_sql.py`).
