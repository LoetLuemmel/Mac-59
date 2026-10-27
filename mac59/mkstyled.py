#!/usr/bin/env python3
"""mkstyled.py TXT OUT.bin [NAME] [--kurs] - a SimpleText document (MacBinary: TEXT/ttxt, data = the text in MacRoman/CR,
'styl' 128) styled as the Mars Lander manual the operator formatted (MarsLander-Anleitung.bin): application font,
title bold 24, headings (lines of capitals) bold, key names, displays and the tried landing bold, the rest plain.
NAME is the document's name (default "Mondlandung Anleitung"). --kurs (the Programmierkurs manual): lesson titles
("2.3 Eigene Funktionstasten"), the indented key lines and the card after "Karte: " bold as well."""
import re, struct, sys

args = [a for a in sys.argv[1:] if a != "--kurs"]
kurs = "--kurs" in sys.argv
text = open(args[0], encoding="utf-8").read().rstrip("\n").replace("\n", "\r")
raw = text.encode("mac_roman")
bold = [False] * len(text)
size = [0] * len(text)
def mark(a, e, s=0):
    for i in range(a, e): bold[i] = True; size[i] = s or size[i]
lines, pos = text.split("\r"), 0
section = ""
for n, ln in enumerate(lines):
    if n == 0: mark(pos, pos + len(ln), 24)
    elif ln and re.fullmatch(r"[A-Z][A-Z0-9 -]*", ln): mark(pos, pos + len(ln)); section = ln
    elif kurs and re.match(r"\d\.\d \S", ln): mark(pos, pos + len(ln))
    elif kurs and ln.startswith("   "): mark(pos + 3, pos + len(ln))
    elif kurs and ln.startswith("Karte: "): mark(pos + 7, pos + len(ln))
    elif section == "TASTEN" and ln.startswith("   "):          # the key column
        m = re.match(r"   (\S+(?: \S+)*?)(?=\s{2,})", ln)
        if m: mark(pos + m.start(1), pos + m.end(1))
    elif section == "ERPROBTE LANDUNG" and ln.startswith("   "): mark(pos + 3, pos + len(ln))
    for m in re.finditer(r"(?<![\w.])(R/S|RST|CLR|RCL 03|A|-?\d+\.\d{4}|0\.0000)(?![\w])", ln):
        if n > 0 and not kurs and not re.fullmatch(r"[A-Z][A-Z0-9 -]*", ln): mark(pos + m.start(), pos + m.end())
    pos += len(ln) + 1
runs = []
for i in range(len(text)):
    k = (bold[i], size[i])
    if not runs or runs[-1][1] != k: runs.append((i, k))
def metrics(sz): return (29, 22) if sz == 24 else (16, 12)    # line height, ascent as in the Mars document
styl = struct.pack(">H", len(runs)) + b"".join(
    struct.pack(">ihhhBBhHHH", st, *metrics(sz), 1, 1 if b else 0, 0, sz, 0, 0, 0) for st, (b, sz) in runs)

def resfork(rtype, rid, data):
    body = struct.pack(">I", len(data)) + data
    tlist = struct.pack(">H", 0) + rtype + struct.pack(">HH", 0, 10)
    refs = struct.pack(">hhI I", rid, -1, 0, 0)
    mapd = b"\0" * 16 + b"\0\0\0\0" + b"\0\0" + struct.pack(">HHH", 0, 28, 28 + len(tlist) + len(refs)) + tlist + refs
    return struct.pack(">IIII", 256, 256 + len(body), len(body), len(mapd)) + b"\0" * 240 + body + mapd
rf = resfork(b"styl", 128, styl)
name = (args[2] if len(args) > 2 else "Mondlandung Anleitung").encode("mac_roman")
hdr = bytearray(128)
hdr[1] = len(name); hdr[2:2 + len(name)] = name
hdr[65:69] = b"TEXT"; hdr[69:73] = b"ttxt"
struct.pack_into(">II", hdr, 83, len(raw), len(rf))
pad = lambda b: b + b"\0" * (-len(b) % 128)
open(args[1], "wb").write(bytes(hdr) + pad(raw) + pad(rf))
print(len(raw), "chars,", len(runs), "style runs")
