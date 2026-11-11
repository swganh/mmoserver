#!/usr/bin/env python3
"""Print a client datatable (.iff, FORM DTII) as CSV.

    python3 -I tools/client/datatable.py datatables/clientregion/tatooine.iff

Reads files extracted from your own client; nothing from the client is
stored in the repository.
"""
import csv
import struct
import sys


def chunks(data, pos, end):
    while pos < end:
        tag = data[pos:pos + 4].decode("ascii")
        size = struct.unpack(">I", data[pos + 4:pos + 8])[0]
        body = pos + 8
        if tag == "FORM":
            yield ("FORM:" + data[body:body + 4].decode("ascii"), body + 4, body + size)
        else:
            yield (tag, body, body + size)
        pos = body + size  # SWG IFF chunks are not padded to even sizes


def find(data, pos, end, path):
    for tag, b, e in chunks(data, pos, end):
        if tag == path[0]:
            return (b, e) if len(path) == 1 else find(data, b, e, path[1:])
        if tag.startswith("FORM:"):
            r = find(data, b, e, path)
            if r:
                return r
    return None


def cstrings(blob):
    return [s.decode("latin-1") for s in blob.split(b"\0")[:-1]]


def read(path):
    data = open(path, "rb").read()
    b, e = find(data, 0, len(data), ["COLS"])
    columns = cstrings(data[b + 4:e])
    b, e = find(data, 0, len(data), ["TYPE"])
    types = cstrings(data[b:e])
    b, e = find(data, 0, len(data), ["ROWS"])
    count = struct.unpack("<I", data[b:b + 4])[0]
    pos, rows = b + 4, []
    for _ in range(count):
        row = []
        for t in types:
            kind = t[0].lower()
            if kind == "s":
                n = data.index(b"\0", pos)
                row.append(data[pos:n].decode("latin-1"))
                pos = n + 1
            elif kind == "f":
                row.append(round(struct.unpack("<f", data[pos:pos + 4])[0], 3))
                pos += 4
            else:  # i, b, e, h: 32-bit ints
                row.append(struct.unpack("<i", data[pos:pos + 4])[0])
                pos += 4
        rows.append(row)
    return columns, rows


if __name__ == "__main__":
    columns, rows = read(sys.argv[1])
    out = csv.writer(sys.stdout)
    out.writerow(columns)
    out.writerows(rows)
