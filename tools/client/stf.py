#!/usr/bin/env python3
"""Print a client string file (.stf) as "name<TAB>text" lines, in id order.

    python3 -I tools/client/stf.py string/en/newbie_tutorial/system_messages.stf

Reads files extracted from your own client; nothing from the client is
stored in the repository.
"""
import struct
import sys


def read(path):
    data = open(path, "rb").read()
    magic, _version, _next_id, count = struct.unpack_from("<IBII", data, 0)
    if magic != 0xABCD:
        raise ValueError("%s: not a string file" % path)
    pos, texts = 13, {}
    for _ in range(count):
        sid, _crc, length = struct.unpack_from("<III", data, pos)
        pos += 12
        texts[sid] = data[pos:pos + length * 2].decode("utf-16-le")
        pos += length * 2
    names = {}
    for _ in range(count):
        sid, length = struct.unpack_from("<II", data, pos)
        pos += 8
        names[sid] = data[pos:pos + length].decode("ascii")
        pos += length
    return [(names[i], texts[i]) for i in sorted(texts)]


if __name__ == "__main__":
    for name, text in read(sys.argv[1]):
        print("%s\t%s" % (name, text.replace("\n", "\\n")))
