# Sources for game content

Content (creatures, places, quests, the tutorial) is recreated from the
client's own files and from material published at the time. The leaked SOE
server source is never a source (`docs/SCRIPTING.md`, "Content sources").

## The client's files

Datatables and string files extracted from a pre-CU client (with an IFF/TRE
tool such as Sytner's IFF Editor) are the first source: they are what the
original game used. `tools/client/` reads them:

| Tool | What |
|---|---|
| `datatable.py <file.iff>` | prints a datatable as CSV |
| `stf.py <file.stf>` | prints a string file as `name<TAB>text` lines |
| `city_regions_sql.py <planet> <clientregion/planet.iff>` | writes a migration with the client's city areas (as `003_tatooine_city_areas.sql`) |
| `skills_compare.py <skill/skills.iff> [--sql]` | compares the client's skill table (Publish 14) with the database's skills; `--sql` writes the fix (as `008_skills_from_client.sql`) |
| `../reference/specials_vs_client.py <cmd_d.txt>` | compares the special moves table (An Empire Divided) with the client's command descriptions (`string/en/cmd_d.stf` through `stf.py`); the contradictions became migration 013 |
| `commands_compare.py <command/command_table.iff> [--sql]` | compares the client's command table (Publish 14) with `command_table`: default times, combat queue use; `--sql` writes the fix (as `009_commands_from_client.sql`) |

The extracted files themselves are not committed: keep them with your
client. Generated data says which client file it came from.

## The guides

Printed strategy guides are the main reference for numbers and places. They
are copyrighted: **never commit the PDFs or copy their text or tables**. Use
the facts (a creature's level, where a quest giver stands) in our own form,
in the data and scripts, with a citation so others can check them.

| Guide | Era | Use it for | Don't use it for |
|---|---|---|---|
| Prima's Official Strategy Guide: *An Empire Divided* (2003, launch) | pre-CU, as the game shipped | combat rules, professions and skills, weapons, armour, creatures, the tutorial | later changes (publishes, the CU) |
| Prima Official Game Guide (2004, the original) | pre-CU, the era this server recreates | creatures, mechanics, skills, planets | post-2004 content |
| The Total Experience (Prima) | post-CU, includes Jump to Lightspeed | places, quests, theme parks, space | mechanics and numbers (changed by the CU and NGE) |

Sections of the 2004 guide (guide page numbers):

| Pages | Section |
|---|---|
| 40-48 | Combat |
| 118-159 | Planets and cities |
| 170-208 | Creatures |

In the PDF of the 2004 guide, the PDF page is the guide page + 1. In the PDF of
*An Empire Divided* (498 pages), the PDF page is the guide page + 3.

Sections of *An Empire Divided* (guide page numbers): New Player
Introduction 5-16, Combat 65-86, Professions 105-320 (Brawler 123,
Marksman 140), Planets 321-376 (Tatooine 324), PvP 377-386, Weapons table
388, Armor table 394, Creature tables 396-483.

## Transcribed reference data

`mmoserverdb/reference/` holds the guides' tables in our own columns, one
row per entry with its printed page, for migrations to take values from:
`prima_ed/` (An Empire Divided: weapons, armour, specials, a few
creatures) and `prima_2004/` (the 2004 guide's creature, droid and NPC
tables, read from the PDF by `tools/reference/prima2004_tables.py`). Each
folder's README says what was transcribed and how it was checked.

## Citing

Cite the guide and the printed page next to the data it supports, e.g. in a
CSV column or a comment:

```
-- Prima 2004, p. 171
```

When two sources disagree, prefer the 2004 guide for anything pre-CU, and
say in the citation which one was used.
