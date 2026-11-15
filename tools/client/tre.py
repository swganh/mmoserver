#!/usr/bin/env python3
"""Read the client's .tre archives directly, without extracting them.

    python3 -I tools/client/tre.py list   <client folder> [pattern]
    python3 -I tools/client/tre.py extract <client folder> <pattern> <out folder>

`pattern` is a shell pattern over the file names inside the archives, e.g.
'datatables/space/*'. When a file is in more than one archive, the one the
client loads last wins: the archive with the highest searchTree priority in
the client's .cfg files (live.cfg, swg2uu_live.cfg, ...), else the later
name in alphabetical order.

Nothing from the client is stored in the repository: extract into a folder
outside it.

The archive layout (TRE version 5000), as read from the client's files:

    header  "EERT" "5000" count, toc_offset, toc_compressor, toc_size,
            names_compressor, names_size, names_uncompressed_size (u32 LE)
    toc     count entries of crc, length, offset, compressor,
            compressed_length, name_offset (u32 LE), at toc_offset
    names   NUL-terminated file names, after the toc
Compressor 0 is none, 2 is zlib.
"""

import fnmatch
import glob
import os
import re
import struct
import sys
import zlib

HEADER = struct.Struct("<4s4s7I")
ENTRY = struct.Struct("<6I")


def _inflate(blob, compressor, size):
    if compressor == 0:
        return blob
    if compressor == 2:
        data = zlib.decompress(blob)
        if len(data) != size:
            raise ValueError("unexpected size after decompressing")
        return data
    raise ValueError("unknown compressor %d" % compressor)


class Archive:
    """One .tre file: its file names and how to read each one."""

    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            header = f.read(HEADER.size)
            (magic, version, count, toc_offset, toc_compressor, toc_size,
             names_compressor, names_size, names_uncompressed) = HEADER.unpack(header)
            if magic not in (b"EERT", b"TREE"):
                raise ValueError("%s is not a TRE archive" % path)
            if version not in (b"5000", b"0005"):
                raise ValueError("%s: TRE version %r is not supported" % (path, version))
            f.seek(toc_offset)
            toc_stored = toc_size if toc_compressor else count * ENTRY.size
            toc = _inflate(f.read(toc_stored), toc_compressor, count * ENTRY.size)
            names_stored = names_size if names_compressor else names_uncompressed
            names = _inflate(f.read(names_stored), names_compressor, names_uncompressed)
        self.entries = {}
        for i in range(count):
            crc, length, offset, compressor, compressed, name_at = ENTRY.unpack_from(toc, i * ENTRY.size)
            name = names[name_at:names.index(b"\0", name_at)].decode("latin-1").lower()
            self.entries[name] = (length, offset, compressor, compressed)

    def read(self, name):
        length, offset, compressor, compressed = self.entries[name]
        with open(self.path, "rb") as f:
            f.seek(offset)
            return _inflate(f.read(compressed if compressor else length), compressor, length)


def _priorities(folder):
    """searchTree_XX_N=archive.tre lines of the client's .cfg files."""
    found = {}
    for cfg in glob.glob(os.path.join(folder, "*.cfg")):
        with open(cfg, encoding="latin-1") as f:
            for line in f:
                m = re.match(r"\s*searchTree_\d+_(\d+)\s*=\s*(\S+)", line)
                if m:
                    found[os.path.basename(m.group(2)).lower()] = int(m.group(1))
    return found


class Client:
    """Every .tre archive in a client folder, the last loaded winning."""

    def __init__(self, folder):
        paths = sorted(glob.glob(os.path.join(folder, "*.tre")), key=lambda p: os.path.basename(p).lower())
        if not paths:
            raise SystemExit("no .tre files in %s" % folder)
        priority = _priorities(folder)
        paths.sort(key=lambda p: priority.get(os.path.basename(p).lower(), -1))
        self.archives = [Archive(p) for p in paths]
        self.files = {}
        for archive in self.archives:  # later archives replace earlier ones
            for name in archive.entries:
                self.files[name] = archive

    def names(self, pattern="*"):
        return sorted(n for n in self.files if fnmatch.fnmatchcase(n, pattern.lower()))

    def read(self, name):
        return self.files[name].read(name)


def main(argv):
    if len(argv) >= 2 and argv[0] == "list":
        client = Client(argv[1])
        for name in client.names(argv[2] if len(argv) > 2 else "*"):
            print("%s\t%s" % (name, os.path.basename(client.files[name].path)))
    elif len(argv) == 4 and argv[0] == "extract":
        client = Client(argv[1])
        out = os.path.abspath(argv[3])
        for name in client.names(argv[2]):
            target = os.path.abspath(os.path.join(out, name))
            if not target.startswith(out + os.sep):
                continue  # a name that would leave the out folder
            os.makedirs(os.path.dirname(target), exist_ok=True)
            with open(target, "wb") as f:
                f.write(client.read(name))
            print(name)
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
