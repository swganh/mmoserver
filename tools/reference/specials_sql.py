#!/usr/bin/env python3
"""Writes the special moves migration from the guide's special moves table.

    python3 tools/reference/specials_sql.py > mmoserverdb/migrations/010_specials_from_guide.sql

Reads mmoserverdb/reference/prima_ed/specials.csv (An Empire Divided,
pp. 76-83) and sets each special's multipliers, to-hit bonus, posture
changes and effects in command_table.
"""

import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CSV = os.path.join(HERE, "..", "..", "mmoserverdb", "reference", "prima_ed", "specials.csv")

# The guide's weapon type -> the prefix of the command's name.
PREFIX = {
    "1H Melee": "melee1h", "2H Melee": "melee2h", "Polearm": "polearm", "Unarmed": "unarmed",
    "Carbine": "", "Pistol": "", "Rifle": "", "Ranged": "",
}
# Names the table gives differently from the command.
RENAMED = {("Pistol", "Defense1"): "pistolMeleeDefense1", ("Pistol", "Defense2"): "pistolMeleeDefense2"}

EFFECTS = {"Stunned": "stun", "Dizzy": "dizzy", "Blinded": "blind"}

HEADER = """\
-- Special moves from Prima's An Empire Divided (2003; docs/SOURCES.md),
-- special moves table (pp. 76-83), transcribed in
-- mmoserverdb/reference/prima_ed/specials.csv. Written by
-- tools/reference/specials_sql.py; edit that, not this.
--
-- For each of the %d specials in the table:
--   damage_multiplier, delay_multiplier   the damage and time modifiers
--   *cost_multiplier                      the HAM cost modifiers (x the
--                                         weapon's attack costs)
--   accuracy_bonus                        the to-hit modifier
--   knockdown, posture down/up chance     100 where the table gives the
--                                         defender's end posture
--                                         (Knocked Down, Down, Up), else 0
--   stun, dizzy, blind chance, duration   the table's effects with their
--                                         chance and duration (seconds)
-- The cost multipliers become FLOAT (the guide has 0.625, 1.25...), and the
-- durations are new columns. The combat rules (data/script/combat/rules.lua)
-- take the defender's defence mods off the chances.
--
-- Not in command_table: the attacker's end posture (Dive Shot: prone, Kip
-- Up Shot: standing, Roll Shot: kneeling) and the area and cone sizes.

ALTER TABLE command_table
  MODIFY COLUMN healthcost_multiplier FLOAT NOT NULL DEFAULT 0,
  MODIFY COLUMN actioncost_multiplier FLOAT NOT NULL DEFAULT 0,
  MODIFY COLUMN mindcost_multiplier FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS stun_duration FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS dizzy_duration FLOAT NOT NULL DEFAULT 0,
  ADD COLUMN IF NOT EXISTS blind_duration FLOAT NOT NULL DEFAULT 0;

"""


def number(text):
    if text is None or text.strip() == "":
        return "0"
    value = float(text)
    return str(int(value)) if value == int(value) else str(value)


def main():
    rows = list(csv.DictReader(open(CSV, newline="")))
    out = sys.stdout
    out.write(HEADER % len(rows))
    for r in rows:
        name = RENAMED.get((r["weapon_type"], r["name"]), PREFIX[r["weapon_type"]] + r["name"])
        posture = r["defender_end_posture"]
        values = {
            "damage_multiplier": number(r["damage_mod"]),
            "delay_multiplier": number(r["time_mod"]),
            "healthcost_multiplier": number(r["health_cost"]),
            "actioncost_multiplier": number(r["action_cost"]),
            "mindcost_multiplier": number(r["mind_cost"]),
            "accuracy_bonus": number(r["to_hit_mod"]),
            "knockdown_chance": "100" if posture == "Knocked Down" else "0",
            "posture_down_chance": "100" if posture == "Down" else "0",
            "posture_up_chance": "100" if posture == "Up" else "0",
            "stun_chance": "0", "stun_duration": "0",
            "dizzy_chance": "0", "dizzy_duration": "0",
            "blind_chance": "0", "blind_duration": "0",
        }
        for i in (1, 2, 3):
            effect = r["effect%d" % i]
            if effect:
                key = EFFECTS[effect]
                values[key + "_chance"] = number(r["effect_chance"])
                values[key + "_duration"] = number(r["effect%d_duration" % i])
        sets = ", ".join("%s = %s" % (k, v) for k, v in values.items())
        out.write("-- %s %s (pp. %s)\nUPDATE command_table SET %s WHERE LOWER(commandname) = LOWER('%s');\n"
                  % (r["weapon_type"], r["name"], r["pages"], sets, name))


if __name__ == "__main__":
    main()
