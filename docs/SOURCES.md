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
| `city_regions_sql.py <planet> <clientregion/planet.iff>` | writes a migration with the client's city areas (as `003_tatooine_city_areas.sql`) |

The extracted files themselves are not committed: keep them with your
client. Generated data says which client file it came from.

## The guides

Printed strategy guides are the main reference for numbers and places. They
are copyrighted: **never commit the PDFs or copy their text or tables**. Use
the facts (a creature's level, where a quest giver stands) in our own form,
in the data and scripts, with a citation so others can check them.

| Guide | Era | Use it for | Don't use it for |
|---|---|---|---|
| Prima Official Game Guide (2004, the original) | pre-CU, the era this server recreates | creatures, mechanics, skills, planets | post-2004 content |
| The Total Experience (Prima) | post-CU, includes Jump to Lightspeed | places, quests, theme parks, space | mechanics and numbers (changed by the CU and NGE) |

Sections of the 2004 guide (guide page numbers):

| Pages | Section |
|---|---|
| 40-48 | Combat |
| 118-159 | Planets and cities |
| 170-208 | Creatures |

In the PDF of the 2004 guide, the PDF page is the guide page + 1.

## Citing

Cite the guide and the printed page next to the data it supports, e.g. in a
CSV column or a comment:

```
-- Prima 2004, p. 171
```

When two sources disagree, prefer the 2004 guide for anything pre-CU, and
say in the citation which one was used.
