#!/usr/bin/env python3
"""Writes the weapon data migration from the guide's weapon table.

    python3 tools/reference/weapons_sql.py > mmoserverdb/migrations/007_weapon_combat_data.sql

Reads mmoserverdb/reference/prima_ed/weapons.csv (An Empire Divided,
pp. 390-393) and maps each weapon to its item type in swganh.item_types.
Weapons not mapped (the lightsaber, grenades) are listed in the output.
"""

import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CSV = os.path.join(HERE, "..", "..", "mmoserverdb", "reference", "prima_ed", "weapons.csv")

# Guide name -> item_types.id (object/weapon/...).
ITEM_TYPES = {
    "dagger (Twi'lek)": 2280,        # melee/knife/knife_dagger
    "gaderiffi baton": 2278,
    "rantok sword": 2668,
    "Ryyk blade": 2303,              # melee/sword/sword_blade_ryyk
    "stone knife": 2630,
    "stone knife (noob)": 2631,
    "stun baton": 2279,
    "survival knife": 2281,
    "sword 01": 2301,
    "sword 01 (noob)": 2651,
    "sword 02": 2302,
    "vibroblade": 2282,
    "battleaxe": 2272,
    "cleaver": 2273,
    "katana": 2274,
    "maul": 2275,
    "heavy duty axe": 2276,          # melee/axe/axe_heavy_duty
    "scythe": 2561,
    "vibro-axe": 2277,               # melee/axe/axe_vibroaxe
    "controllerfp lance": 2288,
    "staff metal lance": 2289,
    "staff wood s1": 2290,
    "staff wood s2": 2291,
    "vibro-axe polearm": 2293,
    "vibrolance": 2292,
    "vibroknuckler": 2294,
    "DH17 carbine": 2308,
    "DH17 snub-nosed carbine": 2309,
    "DXR6 carbine": 2310,
    "E11 carbine": 2311,
    "EE3 carbine": 2312,
    "elite carbine": 2313,
    "laser carbine": 2314,
    "CDEF pistol": 2326,
    "CDEF pistol (noob)": 2755,
    "D18 pistol": 2327,
    "DH17 pistol": 2328,
    "DL44 metal pistol": 2330,
    "DL44 pistol": 2329,
    "DX2 pistol": 2331,
    "FWG5 pistol": 2332,
    "power5 pistol": 2334,
    "scout blaster pistol": 2337,
    "srcombat pistol": 2338,
    "striker pistol (noob)": 2762,
    "striker striker": 2339,
    "tangle pistol": 2340,
    "E11 rifle": 2347,
    "bowcaster rifle": 2343,
    "DLT20 rifle": 2345,
    "DLT20a rifle": 2346,
    "Jawa ion rifle": 2349,
    "laser rifle": 2350,
    "laser rifle (noob)": 2765,
    "SG82 rifle": 2352,
    "spraystick": 2353,
    "T21 rifle": 2354,
    "Tusken rifle": 2355,
}

HEADER = """\
-- Weapon combat data from Prima's An Empire Divided (2003; docs/SOURCES.md),
-- weapons appendix (pp. 390-393, as cited per row), transcribed in
-- mmoserverdb/reference/prima_ed/weapons.csv. Written by
-- tools/reference/weapons_sql.py; edit that, not this.
--
-- 1. Each weapon gets its ideal and max range in metres, as internal
--    attributes (not shown in the examine window): combat_range_mid and
--    combat_range_max. The combat rules (data/script/combat/rules.lua)
--    interpolate the weapon's accuracy modifiers between them.
-- 2. Attack speed, damage, damage type and the three range modifiers take
--    the guide's value where the database holds a single value (not a
--    crafting range): it had empty or "-" values (cleaver, katana, sword
--    02, Jawa ion rifle) and 0 for most pistols' point blank modifier.
--    The same goes for weapons already made from these types.
-- 3. New characters start with the noob CDEF pistol (p. 392: 50-80, 4 s),
--    as new characters did, instead of the plain CDEF at the bottom of its
--    crafting ranges (17-35, 3.2 s).
--
-- Not mapped to an item type: %s.
"""

ATTRIBUTES = [
    # attribute name, csv column, how to write the value
    ("wpn_attack_speed", "attack_speed", "number"),
    ("cat_wpn_damage.wpn_damage_min", "min_damage", "number"),
    ("cat_wpn_damage.wpn_damage_max", "max_damage", "number"),
    ("cat_wpn_damage.wpn_damage_type", "damage_type", "type"),
    ("cat_wpn_rangemods.wpn_range_zero", "zero_range_mod", "number"),
    ("cat_wpn_rangemods.wpn_range_mid", "mid_range_mod", "number"),
    ("cat_wpn_rangemods.wpn_range_max", "max_range_mod", "number"),
]


def number(text):
    value = float(text)
    return str(int(value)) if value == int(value) else str(value)


def sql_string(text):
    return "'" + text.replace("\\", "\\\\").replace("'", "''") + "'"


def main():
    rows = list(csv.DictReader(open(CSV, newline="")))
    mapped = [r for r in rows if r["name"] in ITEM_TYPES]
    unmapped = [r["name"] for r in rows if r["name"] not in ITEM_TYPES]
    out = sys.stdout

    out.write(HEADER % ", ".join(unmapped))
    out.write("""
INSERT IGNORE INTO swganh.attributes (name, internal, attribute_internal_description)
  VALUES ('combat_range_mid', 1, 'Ideal range (m):'),
         ('combat_range_max', 1, 'Max range (m):');

DROP TEMPORARY TABLE IF EXISTS weapons_007;
CREATE TEMPORARY TABLE weapons_007 (
  item_type_id INT UNSIGNED, attribute VARCHAR(64), value VARCHAR(32), page INT,
  PRIMARY KEY (item_type_id, attribute));
INSERT INTO weapons_007 VALUES
""")
    values = []
    for r in mapped:
        type_id = ITEM_TYPES[r["name"]]
        page = int(r["page"])
        values.append("  -- %s (p. %d)" % (r["name"], page))
        for name, column, kind in ATTRIBUTES:
            text = r[column]
            value = text.capitalize() if kind == "type" else number(text)
            values.append("  (%d, %s, %s, %d)," % (type_id, sql_string(name), sql_string(value), page))
        values.append("  (%d, 'combat_range_mid', %s, %d)," % (type_id, sql_string(number(r["mid_range"])), page))
        values.append("  (%d, 'combat_range_max', %s, %d)," % (type_id, sql_string(number(r["max_range"])), page))
    # The last value line ends the statement.
    for i in range(len(values) - 1, -1, -1):
        if values[i].endswith(","):
            values[i] = values[i][:-1] + ";"
            break
    out.write("\n".join(values) + "\n")

    out.write("""
-- 2. Single values (no crafting range) take the guide's.
UPDATE swganh.item_family_attribute_defaults d
  JOIN swganh.attributes a ON a.id = d.attribute_id
  JOIN weapons_007 w ON w.item_type_id = d.item_type_id AND w.attribute = a.name
  SET d.attribute_value = w.value
  WHERE d.attribute_value_max IN ('', '0') OR d.attribute_value_max IS NULL
     OR d.attribute_value_max = d.attribute_value;

UPDATE swganh.item_attributes ia
  JOIN swganh.items i ON i.id = ia.item_id
  JOIN swganh.attributes a ON a.id = ia.attribute_id
  JOIN weapons_007 w ON w.item_type_id = i.item_type AND w.attribute = a.name
  JOIN swganh.item_family_attribute_defaults d
    ON d.item_type_id = i.item_type AND d.attribute_id = ia.attribute_id
  SET ia.value = w.value
  WHERE d.attribute_value_max IN ('', '0') OR d.attribute_value_max IS NULL
     OR d.attribute_value_max = d.attribute_value;

-- 1. Ranges in metres, for new weapons and for those already made.
INSERT INTO swganh.item_family_attribute_defaults
    (family_id, item_type_id, attribute_id, attribute_value, attribute_value_max, attribute_order, attribute_type)
  SELECT 10, w.item_type_id, a.id, w.value, '', 0, 0
  FROM weapons_007 w JOIN swganh.attributes a ON a.name = w.attribute
  WHERE w.attribute IN ('combat_range_mid', 'combat_range_max')
  ON DUPLICATE KEY UPDATE attribute_value = VALUES(attribute_value);

INSERT INTO swganh.item_attributes (item_id, attribute_id, value, `order`)
  SELECT i.id, a.id, w.value, 0
  FROM swganh.items i
  JOIN weapons_007 w ON w.item_type_id = i.item_type
  JOIN swganh.attributes a ON a.name = w.attribute
  WHERE w.attribute IN ('combat_range_mid', 'combat_range_max')
    AND NOT EXISTS (SELECT 1 FROM swganh.item_attributes x
                    WHERE x.item_id = i.id AND x.attribute_id = a.id);

-- 3. New characters: the noob CDEF.
UPDATE swganh.starting_items SET type_id = 2755 WHERE family_id = 10 AND type_id = 2326;

DROP TEMPORARY TABLE weapons_007;
""")


if __name__ == "__main__":
    main()
