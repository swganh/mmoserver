#!/usr/bin/env python3
"""Tests for tre.py and jtl_inventory.py on archives built here.

    python3 -I tools/client/test_tre.py
"""

import io
import os
import struct
import sys
import tempfile
import unittest
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import jtl_inventory  # noqa: E402
import tre  # noqa: E402


def write_tre(path, files, compress=True):
    """A version 5000 archive holding files {name: bytes}."""
    body, entries, names = b"", [], b""
    for name, data in files.items():
        stored = zlib.compress(data) if compress else data
        entries.append((0, len(data), tre.HEADER.size + len(body), 2 if compress else 0,
                        len(stored) if compress else 0, len(names)))
        body += stored
        names += name.encode() + b"\0"
    toc = b"".join(tre.ENTRY.pack(*e) for e in entries)
    toc_stored = zlib.compress(toc) if compress else toc
    names_stored = zlib.compress(names) if compress else names
    header = tre.HEADER.pack(b"EERT", b"5000", len(files), tre.HEADER.size + len(body),
                             2 if compress else 0, len(toc_stored) if compress else 0,
                             2 if compress else 0, len(names_stored), len(names))
    with open(path, "wb") as f:
        f.write(header + body + toc_stored + names_stored)


def iff(tag, body):
    return tag.encode() + struct.pack(">I", len(body)) + body


def datatable(columns, types, rows):
    rows_body = struct.pack("<I", len(rows))
    for row in rows:
        for t, v in zip(types, row):
            rows_body += (v.encode() + b"\0") if t == "s" else struct.pack("<i", v)
    cols = struct.pack("<I", len(columns)) + b"".join(c.encode() + b"\0" for c in columns)
    form = b"0001" + iff("COLS", cols) + iff("TYPE", b"".join(t.encode() + b"\0" for t in types)) \
        + iff("ROWS", rows_body)
    return iff("FORM", b"DTII" + iff("FORM", form))


class TreTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.folder = self.dir.name

    def tearDown(self):
        self.dir.cleanup()

    def test_reads_compressed_and_plain_archives(self):
        for compress in (True, False):
            path = os.path.join(self.folder, "a.tre")
            write_tre(path, {"terrain/space_tatooine.trn": b"x" * 100, "string/en/a.stf": b"abc"}, compress)
            archive = tre.Archive(path)
            self.assertEqual(sorted(archive.entries), ["string/en/a.stf", "terrain/space_tatooine.trn"])
            self.assertEqual(archive.read("terrain/space_tatooine.trn"), b"x" * 100)
            self.assertEqual(archive.read("string/en/a.stf"), b"abc")

    def test_the_archive_loaded_last_wins(self):
        write_tre(os.path.join(self.folder, "patch_01.tre"), {"a.iff": b"new"})
        write_tre(os.path.join(self.folder, "data_01.tre"), {"a.iff": b"old"})
        self.assertEqual(tre.Client(self.folder).read("a.iff"), b"new")  # alphabetical
        with open(os.path.join(self.folder, "live.cfg"), "w") as f:
            f.write("[SharedFile]\n\tsearchTree_00_20=data_01.tre\n\tsearchTree_00_10=patch_01.tre\n")
        self.assertEqual(tre.Client(self.folder).read("a.iff"), b"old")  # by priority

    def test_only_the_listed_archives_are_read(self):
        write_tre(os.path.join(self.folder, "data_00.tre"), {"a.iff": b"p14", "b.iff": b"b"})
        write_tre(os.path.join(self.folder, "patch_30.tre"), {"a.iff": b"later", "c.iff": b"c"})
        listed = os.path.join(self.folder, "publish14.txt")
        with open(listed, "w") as f:
            f.write("# Publish 14\n\nDATA_00.tre\n")
        client = tre.Client(self.folder, tre.read_list(listed))
        self.assertEqual(client.read("a.iff"), b"p14")
        self.assertEqual(client.names(), ["a.iff", "b.iff"])
        self.assertEqual(client.skipped, ["patch_30.tre"])
        out = io.StringIO()
        jtl_inventory.report(client, out)
        self.assertIn("Left out: `patch_30.tre`.", out.getvalue())
        with self.assertRaises(SystemExit):
            tre.Client(self.folder, ["data_00.tre", "missing.tre"])

    def test_load_order_from_a_client_cfg(self):
        cfg = os.path.join(self.folder, "live.cfg")
        with open(cfg, "w") as f:
            f.write('[SharedFile]\n\tmaxSearchPriority=26\n'
                    '\tsearchTree_00_23="G:\\SWG\\a client\\patch_14_00.tre"\n'
                    '\tsearchTree_00_14="G:\\SWG\\a client\\patch_11_01.tre"\n'
                    '\tsearchTree_00_14="G:\\SWG\\a client\\patch_11_00.tre"\n'
                    '\tsearchTree_01_13=data_sku1_00.tre\n'
                    '\tsearchTree_00_0="G:\\SWG\\a client\\bottom.tre"\n')
        self.assertEqual(tre.read_list(cfg), ["bottom.tre", "data_sku1_00.tre", "patch_11_00.tre",
                                              "patch_11_01.tre", "patch_14_00.tre"])

    def test_cfgs_that_disagree_need_archives(self):
        write_tre(os.path.join(self.folder, "a.tre"), {"x.iff": b"a"})
        write_tre(os.path.join(self.folder, "b.tre"), {"x.iff": b"b"})
        with open(os.path.join(self.folder, "live.cfg"), "w") as f:
            f.write("searchTree_00_2=a.tre\nsearchTree_00_1=b.tre\n")
        with open(os.path.join(self.folder, "user.cfg"), "w") as f:
            f.write("[ClientGame]\n")  # no searchTree lines: ignored
        self.assertEqual(tre.Client(self.folder).read("x.iff"), b"a")
        with open(os.path.join(self.folder, "other.cfg"), "w") as f:
            f.write("searchTree_00_1=a.tre\nsearchTree_00_2=b.tre\n")
        with self.assertRaises(SystemExit):
            tre.Client(self.folder)
        self.assertEqual(tre.Client(self.folder, tre.read_list(os.path.join(self.folder, "other.cfg")))
                         .read("x.iff"), b"b")

    def test_rejects_other_files(self):
        path = os.path.join(self.folder, "bad.tre")
        with open(path, "wb") as f:
            f.write(b"\0" * 64)
        with self.assertRaises(ValueError):
            tre.Archive(path)

    def test_inventory(self):
        table = datatable(["name", "mass"], ["s", "i"], [["xwing", 10], ["tie", 5]])
        write_tre(os.path.join(self.folder, "data.tre"), {
            "terrain/space_tatooine.trn": b"t",
            "datatables/space/ship_chassis.iff": table,
            "object/ship/shared_xwing.iff": b"o",
            "object/tangible/food/shared_melon.iff": b"m",
            "misc/spaceship_notes.iff": b"n",
        })
        out = io.StringIO()
        jtl_inventory.report(tre.Client(self.folder), out)
        text = out.getvalue()
        self.assertIn("| Space zones (terrain) | 1 | 0 |", text)
        self.assertIn("`datatables/space/ship_chassis.iff`: 2 rows; name, mass", text)
        self.assertIn("- `object/ship/shared_xwing.iff`", text)
        self.assertIn("- `misc/`: 1", text)
        self.assertNotIn("melon", text)


if __name__ == "__main__":
    unittest.main()
