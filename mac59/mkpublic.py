#!/usr/bin/env python3
"""mkpublic.py PRIVATE.bin PUBLIC.bin [ROMBLOB MODULE] - the public TI-59 app from the private build: the same
MacBinary (both forks, type/creator) without Texas Instruments' firmware, resources 'TROM' 128 (the ROM) and
'TLIB' 128 (the Leisure Library). Every other resource keeps its id, name, attributes and bytes. The public app
reads the ROM from Calc-U-59's text dumps in a folder ROM next to it (port/RomText.h; 'TEXT' 303 says where they
come from). With ROMBLOB and MODULE (build/ti59rom.bin, build/leisure.bin) it also checks that no 64-byte piece of
either is left anywhere in the result. After copying it to the Mac: SetFile -c TI59 -a Bi. The public folder
also gets the folder ROM with the note ROM-Ordner-Hinweis.bin ("ROM-Dateien hier ablegen") - empty of firmware.
Uses AppleBridge's host/macbinary.py (APPLEBRIDGE_HOST, default ~/AppleBridge/host)."""
import os, struct, sys

sys.path.insert(0, os.environ.get("APPLEBRIDGE_HOST", os.path.expanduser("~/AppleBridge/host")))
import macbinary  # noqa: E402

DROP = {(b"TROM", 128), (b"TLIB", 128)}


def read_fork(r):
    """[(type, id, attributes, name pstring or None, data)] in map order, and the map's file attributes"""
    do, mo, dl, ml = struct.unpack(">LLLL", r[:16])
    m = r[mo:mo + ml]
    fattr = struct.unpack(">H", m[22:24])[0]
    tl, nl = struct.unpack(">HH", m[24:28])
    out = []
    for i in range(struct.unpack(">h", m[tl:tl + 2])[0] + 1):
        t, c, off = struct.unpack(">4shh", m[tl + 2 + 8 * i:tl + 10 + 8 * i])
        for j in range(c + 1):
            e = m[tl + off + 12 * j:tl + off + 12 * j + 12]
            rid, no = struct.unpack(">hh", e[:4])
            d = int.from_bytes(e[5:8], "big")
            ln = struct.unpack(">L", r[do + d:do + d + 4])[0]
            name = m[nl + no:nl + no + 1 + m[nl + no]] if no >= 0 else None
            out.append((t, rid, e[4], name, r[do + d + 4:do + d + 4 + ln]))
    return out, fattr


def write_fork(res, fattr):
    data, names, types = bytearray(), bytearray(), []
    for t, rid, attr, name, body in res:
        if not types or types[-1][0] != t:
            types.append((t, []))
        types[-1][1].append((rid, attr, name, len(data)))
        data += struct.pack(">L", len(body)) + body
    tlist, refs = bytearray(struct.pack(">h", len(types) - 1)), bytearray()
    for t, items in types:
        tlist += t + struct.pack(">hh", len(items) - 1, 2 + 8 * len(types) + len(refs))
        for rid, attr, name, doff in items:
            no = -1
            if name is not None:
                no = len(names)
                names += name
            refs += struct.pack(">hh", rid, no) + bytes([attr]) + doff.to_bytes(3, "big") + b"\0" * 4
    body = bytes(tlist + refs)
    hdr = struct.pack(">LLLL", 256, 256 + len(data), len(data), 28 + len(body) + len(names))
    return hdr + b"\0" * 240 + bytes(data) + hdr + b"\0" * 6 + struct.pack(">HHH", fattr, 28, 28 + len(body)) \
        + body + bytes(names)


def main():
    src = macbinary.decode(open(sys.argv[1], "rb").read())
    res, fattr = read_fork(src["rsrc"])
    keep = [x for x in res if (x[0], x[1]) not in DROP]
    gone = sorted(f"{t.decode()} {i}" for t, i, *_ in res if (t, i) in DROP)
    fork = write_fork(keep, fattr)
    assert read_fork(fork)[0] == keep                                    # round trip
    out = macbinary.encode(src["data"], fork, name="Mac-59", type_=src["type"], creator=src["creator"])
    if len(sys.argv) > 4:
        for ref in sys.argv[3:5]:
            blob = open(ref, "rb").read()
            hits = [o for o in range(0, len(blob) - 64, 16) if blob[o:o + 64] in out]
            print(f"{ref}: {len(hits)} of {len(range(0, len(blob) - 64, 16))} pieces found in the public app")
            if hits:
                sys.exit(f"firmware left in {sys.argv[2]}")
    open(sys.argv[2], "wb").write(out)
    print(f"{sys.argv[2]}: removed {', '.join(gone)}; resource fork {len(src['rsrc'])} -> {len(fork)} bytes")


if __name__ == "__main__":
    main()
