#!/usr/bin/env python3
"""Reads the creature, droid and NPC tables of the 2004 Prima guide.

    python3 tools/reference/prima2004_tables.py creatures-part1.pdf creatures-part2.pdf \
        > mmoserverdb/reference/prima_2004/creatures.csv

Takes PDFs of the guide's creature section (printed pp. 170-208; the PDFs
are not in the repository: docs/SOURCES.md) and needs pdftotext (poppler).
Each table runs over a spread: the left page has name, level, location,
social group, HAM, to-hit, damage and XP; the facing page has meat, hide,
bone, milk and notes (creatures) or PvP faction, weapons and notes (NPCs).
Rows on the two pages share their y position, so they are paired by it;
a line on the facing page with no row beside it continues the row above.
Problems (unparsed rows, wrapped names) go to stderr: check those against
the page images.
"""
import csv
import re
import subprocess
import sys

WORD = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>')
LEFT_COLS = ["NAME", "LVL", "LOCATION", "SOCIAL", "HAM", "TO", "DAMAGE", "XP"]


def unescape(s):
    s = "".join(ch for ch in s if ch >= " ")
    return s.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", '"').replace("&apos;", "'")


def pages_of(pdf):
    html = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True, text=True).stdout
    out = []
    for p in html.split("<page ")[1:]:
        words = [(float(a), float(b), float(c), float(d), unescape(w)) for a, b, c, d, w in WORD.findall(p)]
        num = None
        for x0, y0, x1, y1, w in words:
            if round(y0) == 3 and w.isdigit():
                num = int(w)
        out.append((num, words))
    return out


def rows_of(words):
    rows = {}
    for x0, y0, x1, y1, w in words:
        rows.setdefault(round(y0), []).append((x0, x1, w))
    return {y: sorted(r) for y, r in rows.items()}


SIDEBAR = {"Stats:", "Creatures", "Creatures,", "&", "NPCs", "Droids", "Droids,", "Creatures &"}
SECTIONS = {"Creatures", "Androids & Droids", "NPCs"}
PLACES = ["Yavin 4", "Tatooine", "Naboo", "Corellia", "Talus", "Rori", "Dantooine", "Lok", "Endor",
          "Dathomir", "anywhere", "Anywhere", "Nowhere", "nowhere", "global", "Global", "Kashyyyk"]


def drop_sidebar(r, limit):
    return [(x0, x1, w) for x0, x1, w in r if not (w in SIDEBAR and x1 - x0 < 12 and (x0 < limit[0] or x0 > limit[1]))]


def left_tables(words):
    rows = rows_of(words)
    result = []
    hx = None
    section = None
    for y in sorted(rows):
        if y < 100 or y > 538:
            continue
        r = rows[y]
        if any(w == "NAME" for _, _, w in r) and any(w == "LVL" for _, _, w in r):
            hx = {}
            for x0, x1, w in r:
                hx.setdefault(w, x0)
            continue
        if hx is None:
            t = " ".join(w for x0, x1, w in r if x0 > 75)
            if t in SECTIONS:
                section = t
            continue
        r = drop_sidebar(r, (hx["NAME"] - 3, 10000))
        if not r:
            continue
        text = " ".join(w for _, _, w in r)
        if not any(w.endswith("%") for _, _, w in r):
            if text in SECTIONS:
                section = text
            else:
                result.append((y, section, {"wrap": text, "wrap_in_name": r[0][0] < hx["LVL"] - 4}))
            continue
        name = " ".join(w for x0, x1, w in r if x0 < hx["LVL"] - 4)
        rest = " ".join(w for x0, x1, w in r if x0 >= hx["LVL"] - 4)
        m = re.match(r"^(?:\S+\s+)?(\d+)\s+(.*?)\s*([\d.,K]+\s*[–-]\s*[\d.,K]+)\s+(\d+)%\s+"
                     r"([\d.,]+\s*[–-]\s*[\d.,]+)\s+([\d,]+)$", rest)
        if not m:
            print("unparsed p? %s | %s" % (name, rest), file=sys.stderr)
            continue
        lvl, midt, ham, tohit, dmg, xp = m.groups()
        loc = ""
        for pl in PLACES:
            if midt == pl or midt.startswith(pl + " "):
                loc = pl
                break
        group = midt[len(loc):].strip()
        result.append((y, section, {"name": name, "lvl": lvl, "location": loc,
                                    "group": group, "ham": ham, "tohit": tohit + "%",
                                    "damage": dmg, "xp": xp.replace(",", "")}))
    return result


def attach_qty(r):
    """Joins "(n)" to the word before it so a quantity stays with its type."""
    out = []
    for x0, x1, w in r:
        if out and w.startswith("(") and x0 - out[-1][1] < 6:
            px0, px1, pw = out[-1]
            out[-1] = (px0, x1, pw + " " + w)
        else:
            out.append((x0, x1, w))
    return out


def right_rows(words):
    rows = rows_of(words)
    out = []
    hx = None
    kind = None
    for y in sorted(rows):
        if y < 100 or y > 538:
            continue
        r = rows[y]
        text = " ".join(w for _, _, w in r)
        if text.startswith("MEAT HIDE") or text.startswith("PVP FACTION"):
            hx = {}
            for x0, x1, w in r:
                hx.setdefault(w, x0)
            kind = "creature" if text.startswith("MEAT") else "npc"
            continue
        if hx is None:
            continue
        r = drop_sidebar(r, (0, hx["NOTES"] + 60))
        r = attach_qty(r)
        if not r:
            continue
        f = {}
        if kind == "creature":
            bounds = [("meat", hx["HIDE"] - 15), ("hide", hx["BONE"] - 15), ("bone", hx["MILK"] - 8),
                      ("milk", hx["NOTES"] - 3), ("notes", 10000)]
        else:
            bounds = None
        if bounds:
            for x0, x1, w in r:
                for name, lim in bounds:
                    if x0 < lim:
                        f[name] = (f.get(name, "") + " " + w).strip()
                        break
        else:
            notes = [w for x0, x1, w in r if x0 >= hx["NOTES"] - 3]
            rest = [(x0, x1, w) for x0, x1, w in r if x0 < hx["NOTES"] - 3]
            weapons, pvp = [], []
            # weapons are centred under WEAPONS; the PvP faction is the
            # first word(s) at the left of the column
            for x0, x1, w in rest:
                if x0 < hx["WEAPONS"] - 50 and not weapons:
                    pvp.append(w)
                else:
                    weapons.append(w)
            f = {"pvp": " ".join(pvp), "weapons": " ".join(weapons), "notes": " ".join(notes)}
        out.append((y, kind, f))
    return out


def main():
    last_section = ["Creatures"]
    writer = csv.writer(sys.stdout, lineterminator="\n")
    writer.writerow(["page", "facing", "section", "name", "level", "location", "social_group",
                     "ham_min", "ham_max", "to_hit", "damage_min", "damage_max", "xp",
                     "meat", "meat_qty", "hide", "hide_qty", "bone", "bone_qty", "milk",
                     "pvp_faction", "weapons", "notes"])
    for pdf in sys.argv[1:]:
        pages = pages_of(pdf)
        i = 0
        while i + 1 < len(pages):
            lnum, lwords = pages[i]
            rnum, rwords = pages[i + 1]
            left = left_tables(lwords)
            if not left or lnum is None or lnum % 2:
                i += 1
                continue
            right = right_rows(rwords)
            rmap = {y: (k, f) for y, k, f in right}
            used = set()
            recs = []
            merged = []
            for y, section, f in left:
                if "wrap" in f:
                    if merged:
                        key = "name" if f["wrap_in_name"] else "group"
                        merged[-1][2][key] += " " + f["wrap"]
                        print("wrapped %s p%d: %s" % (key, lnum, merged[-1][2][key]), file=sys.stderr)
                    continue
                merged.append((y, section, f))
            fixed = []
            for y, section, f in merged:
                section = section or last_section[0]
                last_section[0] = section
                fixed.append((y, section, f))
            merged = fixed
            for y, section, f in merged:
                rk, rf = rmap.get(y, (None, {}))
                if y in rmap:
                    used.add(y)
                recs.append([y, section, f, rk, dict(rf)])
            # continuation lines on the right page: append to the row above
            for y, k, f in right:
                if y in used:
                    continue
                above = [rec for rec in recs if rec[0] < y]
                if not above:
                    print("unmatched right row p%d y%d %s" % (rnum, y, f), file=sys.stderr)
                    continue
                rec = above[-1]
                for key, val in f.items():
                    if val:
                        rec[4][key] = (rec[4].get(key, "") + " " + val).strip()
            for y, section, f, rk, rf in recs:
                def rng(s):
                    s = s.replace("K", "000")
                    m = re.match(r"^([\d.]+)\s*[–-]\s*([\d.]+)$", s.strip())
                    return (m.group(1), m.group(2)) if m else (s.strip(), s.strip())
                def qty(s):
                    m = re.match(r"^(.*?)\s*\((\d*)\)\s*$", s.strip())
                    return (m.group(1), m.group(2)) if m else (s.strip(), "")
                hmin, hmax = rng(f["ham"])
                dmin, dmax = rng(f["damage"])
                meat, mq = qty(rf.get("meat", ""))
                hide, hq = qty(rf.get("hide", ""))
                bone_text = rf.get("bone", "")
                milk_text = rf.get("milk", "")
                mm = re.search(r"\s*(milk(?: \(wild\))?)\s*$", bone_text)
                if mm:
                    milk_text = (mm.group(1) + " " + milk_text).strip()
                    bone_text = bone_text[:mm.start()]
                rf["milk"] = milk_text
                bone, bq = qty(bone_text)
                writer.writerow([lnum, rnum, section or "Creatures", f["name"], f["lvl"], f["location"], f["group"],
                                 hmin, hmax, f["tohit"].rstrip("%"), dmin, dmax, f["xp"],
                                 meat, mq, hide, hq, bone, bq, rf.get("milk", ""),
                                 rf.get("pvp", ""), rf.get("weapons", ""), rf.get("notes", "")])
            i += 2


main()
