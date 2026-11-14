#!/usr/bin/env python3
"""Writes the specials' areas, cones and attacker postures migration.

    python3 tools/reference/special_areas_sql.py > mmoserverdb/migrations/012_special_areas_from_guide.sql

Reads mmoserverdb/reference/prima_ed/specials.csv (An Empire Divided,
pp. 76-83) for what migration 010 left out: which specials hit an area or a
cone and how big it is, and the attacker's end posture.
"""

import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from specials_sql import CSV, PREFIX, RENAMED, number  # noqa: E402

POSTURES = {"Prone": "prone", "Kneeling": "kneeling", "Standing": "standing"}

HEADER = """\
-- Specials' areas, cones and attacker postures from Prima's An Empire
-- Divided (2003; docs/SOURCES.md), special moves table (pp. 76-83),
-- transcribed in mmoserverdb/reference/prima_ed/specials.csv. Written by
-- tools/reference/special_areas_sql.py; edit that, not this.
--
-- New command_table columns:
--   area_shape            single, area or cone. "Area affects all targets
--                         within the melee weapon's range. Cone affects all
--                         targets with the ranged weapon's cone of fire"
--                         (p. 75).
--   area_length           metres: an area's radius around the attacker, a
--                         cone's length; 0 = the weapon's max range.
--   area_width            metres: how wide a cone is at its far end.
--   attacker_end_posture  the posture the attacker ends in (Dive Shot:
--                         prone, Kip Up Shot: standing, Roll Shot: kneeling),
--                         empty for the rest.
--
-- %d of the table's %d rows are areas or cones or move the attacker.
-- PointBlankArea1 is the one area with a size, 12 by 15: its 12 m is taken
-- as the radius.

ALTER TABLE command_table
  ADD COLUMN IF NOT EXISTS area_shape VARCHAR(8) NOT NULL DEFAULT 'single',
  ADD COLUMN IF NOT EXISTS area_length FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS area_width FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS attacker_end_posture VARCHAR(16) NOT NULL DEFAULT '';

"""


def main():
    rows = list(csv.DictReader(open(CSV, newline="")))
    lines = []
    for r in rows:
        shape = r["targets"].lower()
        posture = POSTURES.get(r["attacker_end_posture"], "")
        if shape == "single" and not posture:
            continue
        name = RENAMED.get((r["weapon_type"], r["name"]), PREFIX[r["weapon_type"]] + r["name"])
        length = "0" if r["area_length"] in ("", "weapon") else number(r["area_length"])
        width = number(r["area_width"]) if shape == "cone" else "0"
        lines.append("-- %s %s: %s (pp. %s)\nUPDATE command_table SET area_shape = '%s', area_length = %s, "
                     "area_width = %s, attacker_end_posture = '%s' WHERE LOWER(commandname) = LOWER('%s');\n"
                     % (r["weapon_type"], r["name"], r["description"], r["pages"], shape, length, width,
                        posture, name))
    out = sys.stdout
    out.write(HEADER % (len(lines), len(rows)))
    out.write("".join(lines))


if __name__ == "__main__":
    main()
