#!/usr/bin/env python3
"""Compares the special moves table with the client's command descriptions.

    python3 tools/client/stf.py string/en/cmd_d.stf > cmd_d.txt
    python3 tools/reference/specials_vs_client.py cmd_d.txt

Reads mmoserverdb/reference/prima_ed/specials.csv (An Empire Divided,
pp. 76-83) and the client's command descriptions (string/en/cmd_d.stf, as
`name<TAB>text` lines from tools/client/stf.py), and prints each special
whose description names an effect, an area or a cone the table does not
have, or the other way round. Many descriptions are only vague ("more
damage than..."), so read each line: only a description that says
something the table contradicts is a correction (migration 013).
"""

import csv
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from specials_sql import CSV, PREFIX, RENAMED  # noqa: E402


def effects(row):
    return {row["effect1"], row["effect2"], row["effect3"]}


CHECKS = [
    ("knockdown", lambda d: bool(re.search(r"knock(s|ing)? (\w+ )?(\w+ )?down|knockdown", d)),
     lambda r: r["defender_end_posture"] == "Knocked Down"),
    ("posture down", lambda d: "lower" in d and "posture" in d,
     lambda r: r["defender_end_posture"] == "Down"),
    ("blind", lambda d: "blind" in d, lambda r: "Blinded" in effects(r)),
    ("dizzy", lambda d: "dizzy" in d, lambda r: "Dizzy" in effects(r)),
    ("stun", lambda d: bool(re.search(r"\bstun", d)), lambda r: "Stunned" in effects(r)),
    ("area or cone", lambda d: bool(re.search(r"\barea\b|\bcone\b|multiple targets", d)),
     lambda r: r["targets"] in ("Area", "Cone")),
]


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    descriptions = {}
    with open(sys.argv[1], encoding="utf-8") as f:
        for line in f:
            if "\t" in line:
                key, text = line.rstrip("\n").split("\t", 1)
                descriptions[key.lower()] = text
    seen = set()
    with open(CSV, newline="") as f:
        for row in csv.DictReader(f):
            name = RENAMED.get((row["weapon_type"], row["name"]), PREFIX[row["weapon_type"]] + row["name"]).lower()
            if name in seen:
                continue
            seen.add(name)
            text = descriptions.get(name)
            if text is None:
                print("%-22s no description" % name)
                continue
            for what, says, has in CHECKS:
                if says(text.lower()) != has(row):
                    print("%-22s %-13s client %-3s table %-3s | %s" % (
                        name, what, "yes" if says(text.lower()) else "no",
                        "yes" if has(row) else "no", text[:120]))


if __name__ == "__main__":
    main()
