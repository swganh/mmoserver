#!/usr/bin/env python3
"""Writes the creature stats migration from the guides' creature tables.

    python3 tools/reference/creatures_sql.py > mmoserverdb/migrations/011_creatures_from_guides.sql

Reads mmoserverdb/reference/prima_2004/creatures.csv (the 2004 guide,
levels 1-11) and mmoserverdb/reference/prima_ed/creatures.csv (An Empire
Divided, for the creatures above level 11), and sets each spawned
creature's (non_persistent_npcs) level, HAM, to-hit, damage, XP, behaviour
flags, social group and harvestable resources.
"""

import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REF = os.path.join(HERE, "..", "..", "mmoserverdb", "reference")

# non_persistent_npcs.id -> (guide, name in its table). The 2004 guide
# first; An Empire Divided for creatures the 2004 table leaves out.
CREATURES = [
    (101, "2004", "womp rat"),
    (102, "ed", "greater desert womprat"),
    (103, "2004", "lesser desert womp rat"),
    (104, "ed", "variegated womprat"),
    (105, "2004", "womp rat"),      # shared_womp_rat_hue.iff: a womp rat in another colour
    (106, "2004", "rill"),
]

# Attributes this migration adds (all internal: not shown on examine).
NEW_ATTRIBUTES = [
    ("creature_ham_min", "Lowest health, action and mind a spawn rolls (guide HAM range)"),
    ("creature_ham_max", "Highest health, action and mind a spawn rolls (guide HAM range)"),
    ("creature_social_group", "Social group: pack creatures assist their own"),
    ("creature_meat_qty", "Meat a scout harvests"),
    ("creature_hide_qty", "Hide a scout harvests"),
    ("creature_bone_qty", "Bone a scout harvests"),
    ("creature_milk", "Milk: milk, wild, or empty"),
]

POOLS = {
    "health": ["creature_health", "creature_strength", "creature_constitution"],
    "action": ["creature_action", "creature_quickness", "creature_stamina"],
    "mind": ["creature_mind", "creature_focus", "creature_willpower"],
}


def load(path):
    with open(path, newline="") as f:
        return {row["name"]: row for row in csv.DictReader(f)}


def flags_2004(notes):
    words = [w.strip() for w in notes.split(",")]
    return {name: name in words for name in ("Aggro", "Killer", "Stalker", "Pack", "Herd", "Tamable")}


def resource(kind, value):
    return "%s %s" % (value.capitalize(), kind) if value else ""


def sql_str(s):
    return "'" + str(s).replace("\\", "\\\\").replace("'", "\\'") + "'"


def main():
    p2004 = load(os.path.join(REF, "prima_2004", "creatures.csv"))
    ped = load(os.path.join(REF, "prima_ed", "creatures.csv"))
    rows = []      # (npc_id, attribute, value)
    levels = []
    table = []
    for npc_id, guide, name in CREATURES:
        if guide == "2004":
            r = p2004[name]
            f = flags_2004(r["notes"])
            ham = {pool: (int(r["ham_min"]), int(r["ham_max"])) for pool in POOLS}
            cite = "Prima 2004, pp. %s-%s" % (r["page"], r["facing"])
            values = {
                "creature_tohit": r["to_hit"],
                "creature_damage_min": r["damage_min"], "creature_damage_max": r["damage_max"],
                "creature_xp": r["xp"],
                "creature_social_group": r["social_group"],
                "killer": int(f["Killer"]), "Pack": int(f["Pack"]), "Herd": int(f["Herd"]),
                "stalking": int(f["Stalker"]), "creature_is_aggressive": int(f["Aggro"]),
                "creature_group_assist": int(f["Pack"]),
                "res_meat": resource("Meat", r["meat"]), "creature_meat_qty": r["meat_qty"] or 0,
                "res_hide": resource("Hide", r["hide"]), "creature_hide_qty": r["hide_qty"] or 0,
                "res_bone": resource("Bone", r["bone"]), "creature_bone_qty": r["bone_qty"] or 0,
                "creature_milk": r["milk"].replace("milk (wild)", "wild"),
            }
            flags = r["notes"]
        else:
            r = ped[name]
            yes = lambda k: int(r[k] == "yes")
            ham = {pool: (int(r[pool + "_min"]), int(r[pool + "_max"])) for pool in POOLS}
            cite = "An Empire Divided, pp. %s-%s" % (r["page"], r["facing"])
            values = {
                "creature_tohit": r["to_hit"],
                "creature_damage_min": r["damage_min"], "creature_damage_max": r["damage_max"],
                "creature_xp": r["xp"],
                "creature_social_group": r["social_group"],
                "killer": yes("killer"), "Pack": yes("pack"), "Herd": yes("herd"),
                "stalking": yes("stalker"), "creature_is_aggressive": yes("aggro"),
                "creature_group_assist": yes("pack"),
            }
            flags = ", ".join(k.capitalize() for k in ("aggro", "killer", "stalker", "pack", "herd")
                              if r[k] == "yes") + (", Tamable" if r["can_tame"] == "yes" else "")
            flags += "".join(", " + a for a in (r["special_attack_1"], r["special_attack_2"]) if a)
        levels.append((npc_id, r["level"]))
        for pool, attrs in POOLS.items():
            lo, hi = ham[pool]
            for a in attrs:
                rows.append((npc_id, a, (lo + hi) // 2))
        rows.append((npc_id, "creature_ham_min", ham["health"][0]))
        rows.append((npc_id, "creature_ham_max", ham["health"][1]))
        for a, v in values.items():
            if v != "":
                rows.append((npc_id, a, v))
        table.append("--   %-4d %-24s %-3s %-10s %3s%%  %-8s %-5s %s" % (
            npc_id, name, r["level"], "%d-%d" % ham["health"], r["to_hit"],
            "%s-%s" % (r["damage_min"], r["damage_max"]), r["xp"], cite))
        table.append("--        %s" % flags)

    out = sys.stdout
    out.write("""\
-- Creature stats from the guides' creature tables (docs/SOURCES.md),
-- transcribed in mmoserverdb/reference/prima_2004/creatures.csv (the 2004
-- guide, levels 1-11) and mmoserverdb/reference/prima_ed/creatures.csv
-- (An Empire Divided, 2003, for creatures above level 11). Written by
-- tools/reference/creatures_sql.py; edit that, not this.
--
-- For each spawned creature: level; each pool the middle of the guide's
-- HAM range, and the range itself (creature_ham_min/max) for spawns to
-- roll in; to-hit (creature_tohit, %); damage; XP; the behaviour flags
-- (aggressive, killer, stalker, pack, herd; pack creatures assist their
-- social group); and, from the 2004 guide, meat, hide and bone with the
-- quantities a scout harvests, and milk.
--
-- Neither guide marks the womp rats or the rill aggressive: they were
-- aggressive here, and are not any more.
--
--   id   creature                 lvl HAM        to-hit damage  XP    source
""")
    out.write("\n".join(table) + "\n--\n")
    out.write("-- The greater desert and variegated womp rats keep their meat, hide and\n"
              "-- bone: An Empire Divided's table has no resources.\n\n")

    out.write("INSERT IGNORE INTO swganh.attributes (name, internal, attribute_internal_description)\n  VALUES ")
    out.write(",\n         ".join("(%s, 1, %s)" % (sql_str(n), sql_str(d)) for n, d in NEW_ATTRIBUTES))
    out.write(";\n\n")

    out.write("DROP TEMPORARY TABLE IF EXISTS creatures_011;\n"
              "CREATE TEMPORARY TABLE creatures_011 (\n"
              "  npc_id BIGINT UNSIGNED, attribute VARCHAR(64), value VARCHAR(64),\n"
              "  PRIMARY KEY (npc_id, attribute));\n"
              "INSERT INTO creatures_011 VALUES\n")
    out.write(",\n".join("  (%d, %s, %s)" % (i, sql_str(a), sql_str(v)) for i, a, v in rows))
    out.write(";\n\n")

    out.write("UPDATE swganh.non_persistent_npcs SET level = CASE id\n")
    out.write("".join("    WHEN %d THEN %s\n" % (i, lv) for i, lv in levels))
    out.write("    ELSE level END\n  WHERE id IN (%s);\n\n" % ", ".join(str(i) for i, _ in levels))

    out.write("""\
-- Existing attributes keep their place in the examine window; new ones are
-- added (internal ones are not shown there).
INSERT INTO swganh.non_persistent_npc_attributes (npc_id, attribute_id, value, `order`)
  SELECT c.npc_id, a.id, c.value, 0
  FROM creatures_011 c JOIN swganh.attributes a ON a.name = c.attribute
  JOIN swganh.non_persistent_npcs n ON n.id = c.npc_id
  ON DUPLICATE KEY UPDATE value = VALUES(value);

DROP TEMPORARY TABLE creatures_011;
""")


main()
