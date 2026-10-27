#!/usr/bin/env python3
"""mkbundle.py [preview.png] - writes Bundle.r: Mac-59's Finder bundle (signature 'TI59', BNDL, FREF) and
its two icon families, drawn here pixel by pixel: 128 the calculator (the app), 129 a magnetic card (its documents).
FREF 128 APPL, 129 'TICD' (binary cards the app writes), 130 'TEXT' (Calc-U-59 text cards); a TEXT file shows the
card only with creator 'TI59' - the Finder picks icons by creator AND type.
Rez -a Bundle.r -o "Project Resources.rsrc", then build; after Build Application: SetFile -c TI59 -a Bi Mac-59.
Colours: indices of the Mac's standard 8-bit table (the 6x6x6 cube, 0 = white, 255 = black) and of the 4-bit table."""
import sys

# ---- drawing: a grid of colour names -------------------------------------------------------------

def canvas(n):
    return [[None] * n for _ in range(n)]


def rect(c, x0, y0, x1, y1, col):                       # inclusive corners
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            c[y][x] = col


def frame(c, x0, y0, x1, y1, col):
    for x in range(x0, x1 + 1):
        c[y0][x] = c[y1][x] = col
    for y in range(y0, y1 + 1):
        c[y][x0] = c[y][x1] = col


DIGITS = {"5": ["111", "100", "111", "001", "111"], "9": ["111", "101", "111", "001", "111"]}


def text(c, x, y, s, col):                               # 3x5 digits
    for k, ch in enumerate(s):
        for dy, row in enumerate(DIGITS[ch]):
            for dx, bit in enumerate(row):
                if bit == "1":
                    c[y + dy][x + 4 * k + dx] = col


def calculator(n):
    """the TI-59 seen from the front, lit from the top left like the System 7 icons: a black outline, a lighter
    top/left bevel and a dark bottom/right one; the red LED display in its window; the key field (top rows dark,
    digits light) and a drop shadow"""
    c = canvas(n)
    if n == 32:
        x0, x1, y0, y1 = 6, 24, 0, 30
        rect(c, x0, y0, x1, y1, "case")
        rect(c, x0 + 1, y0 + 1, x1 - 1, y0 + 1, "bevel")    # light top and left
        rect(c, x0 + 1, y0 + 1, x0 + 1, y1 - 1, "bevel")
        rect(c, x1 - 1, y0 + 2, x1 - 1, y1 - 1, "deep")     # dark right and bottom
        rect(c, x0 + 2, y1 - 1, x1 - 1, y1 - 1, "deep")
        frame(c, x0, y0, x1, y1, "black")
        rect(c, x1 + 1, y0 + 2, x1 + 1, y1 + 1, "shadow")   # drop shadow
        rect(c, x0 + 2, y1 + 1, x1 + 1, y1 + 1, "shadow")
        frame(c, x0 + 3, 3, x1 - 3, 8, "bezel")             # display window
        rect(c, x0 + 4, 4, x1 - 4, 7, "window")
        for x in (x0 + 5, x0 + 8, x0 + 11):                 # three LED digits "8." in red
            rect(c, x, 5, x + 1, 6, "led")
        c[6][x0 + 14] = "led"
        rect(c, x0 + 3, 10, x1 - 3, 10, "brass")
        for row, y in enumerate(range(12, 29, 2)):
            for col, x in enumerate(range(x0 + 3, x1 - 2, 3)):
                k = "keylight" if (row >= 4 and 1 <= col <= 3) else "keydark"
                rect(c, x, y, x + 1, y, k)
    else:
        x0, x1, y0, y1 = 3, 12, 0, 14
        rect(c, x0, y0, x1, y1, "case")
        frame(c, x0, y0, x1, y1, "black")
        rect(c, x1 + 1, y0 + 1, x1 + 1, y1 + 1, "shadow")
        rect(c, x0 + 1, y1 + 1, x1 + 1, y1 + 1, "shadow")
        rect(c, x0 + 1, 1, x1 - 1, 1, "bevel")
        rect(c, x0 + 2, 2, x1 - 2, 4, "window")
        for x in (x0 + 3, x0 + 5, x0 + 7):
            c[3][x] = "led"
        for row, y in enumerate(range(6, 14, 2)):
            for col, x in enumerate(range(x0 + 2, x1 - 1, 2)):
                c[y][x] = "keylight" if (row >= 2 and 1 <= col <= 2) else "keydark"
    return c


def card(n):
    """a document in the System 7 manner (dog-ear with a grey fold, black outline, shadow inside right and
    bottom) carrying a magnetic card's marks: the brown stripe, "59" in red, the label's lines"""
    c = canvas(n)
    if n == 32:
        x0, x1, y0, y1, e = 4, 27, 0, 31, 7                 # e: the dog-ear's size
        rect(c, x0, y0, x1, y1, "white")
        for i in range(e):                                  # cut the corner
            for x in range(x1 - e + 1 + i, x1 + 1):
                c[y0 + i][x] = None
        for x in range(x0, x1 - e + 1):
            c[y0][x] = "black"
        for y in range(y0, y1 + 1):
            c[y][x0] = "black"
        for y in range(y0 + e, y1 + 1):
            c[y][x1] = "black"
        for x in range(x0, x1 + 1):
            c[y1][x] = "black"
        for i in range(e + 1):                              # the diagonal and the fold
            c[y0 + i][x1 - e + i] = "black"
        for y in range(y0, y0 + e + 1):
            c[y][x1 - e] = "black"
        for x in range(x1 - e, x1 + 1):
            c[y0 + e][x] = "black"
        for i in range(1, e):
            for x in range(x1 - e + 1, x1 - e + i + 1):
                if c[y0 + i][x] != "black":
                    c[y0 + i][x] = "fold"
        rect(c, x1 - 1, y0 + e + 1, x1 - 1, y1 - 1, "fold")  # inner shadow
        rect(c, x0 + 1, y1 - 1, x1 - 1, y1 - 1, "fold")
        rect(c, x0 + 3, 3, x0 + 6, 27, "stripe")             # magnetic stripe, darker edge
        rect(c, x0 + 6, 3, x0 + 6, 27, "stripedeep")
        text(c, x0 + 9, 3, "59", "led")
        for k, y in enumerate(range(11, 28, 3)):
            rect(c, x0 + 9, y, x1 - (4 if k % 2 else 3), y, "line")
    else:
        x0, x1, y0, y1, e = 2, 13, 0, 15, 4
        rect(c, x0, y0, x1, y1, "white")
        for i in range(e):
            for x in range(x1 - e + 1 + i, x1 + 1):
                c[y0 + i][x] = None
        for x in range(x0, x1 - e + 1):
            c[y0][x] = "black"
        for y in range(y0, y1 + 1):
            c[y][x0] = "black"
        for y in range(y0 + e, y1 + 1):
            c[y][x1] = "black"
        for x in range(x0, x1 + 1):
            c[y1][x] = "black"
        for i in range(e + 1):
            c[y0 + i][x1 - e + i] = "black"
        for y in range(y0, y0 + e + 1):
            c[y][x1 - e] = "black"
        for x in range(x1 - e, x1 + 1):
            c[y0 + e][x] = "black"
        for i in range(1, e):
            for x in range(x1 - e + 1, x1 - e + i + 1):
                if c[y0 + i][x] != "black":
                    c[y0 + i][x] = "fold"
        rect(c, x0 + 2, 2, x0 + 3, 13, "stripe")
        for y in (5, 7, 9, 11, 13):
            rect(c, x0 + 5, y, x1 - 2, y, "line")
    return c


# ---- colours ---------------------------------------------------------------------------------------
def cube(r, g, b):                                        # 0..5 each, 5 = full; black is 255, not in the cube
    assert (r, g, b) != (0, 0, 0), "black is index 255"
    return (5 - r) * 36 + (5 - g) * 6 + (5 - b)


C8 = {"black": 255, "white": 0, "case": cube(1, 1, 1), "bevel": cube(2, 2, 2), "deep": 255,
      "shadow": cube(3, 3, 3), "bezel": cube(2, 2, 2), "window": 255, "led": cube(5, 0, 0), "brass": cube(4, 3, 1),
      "keydark": cube(2, 2, 2), "keylight": cube(4, 4, 4), "stripe": cube(2, 1, 0), "stripedeep": cube(1, 0, 0),
      "fold": cube(4, 4, 4), "line": cube(1, 1, 1)}
C4 = {"black": 15, "white": 0, "case": 14, "bevel": 13, "deep": 15, "shadow": 12, "bezel": 13, "window": 15,
      "led": 3, "brass": 1, "keydark": 13, "keylight": 11, "stripe": 10, "stripedeep": 15, "fold": 11, "line": 15}
                                                         # 4-bit table: 1 yellow 3 red 10 brown 11-14 light to dark gray
ONE = {"white": 0, "keylight": 0, "led": 0, "fold": 0, "shadow": 0}               # 1-bit: these are white, the rest black


def rgb_of_cube(i):
    if i == 255:
        return (0, 0, 0)
    return (0x33 * (5 - i // 36), 0x33 * (5 - (i // 6) % 6), 0x33 * (5 - i % 6))


def bits(c):
    n = len(c)
    icon, mask = bytearray(), bytearray()
    for plane, test in ((icon, lambda v: v is not None and ONE.get(v, 1)), (mask, lambda v: v is not None)):
        for y in range(n):
            for x0 in range(0, n, 8):
                b = 0
                for x in range(x0, x0 + 8):
                    b = b << 1 | (1 if test(c[y][x]) else 0)
                plane.append(b)
    return bytes(icon + mask)


def deep(c, table, per):
    out = bytearray()
    for row in c:
        vals = [table.get(v, 0) if v is not None else 0 for v in row]
        if per == 8:
            out += bytes(vals)
        else:
            out += bytes(vals[i] << 4 | vals[i + 1] for i in range(0, len(vals), 2))
    return bytes(out)


def hexdata(b, width=32):
    lines = []
    for i in range(0, len(b), width):
        lines.append('\t$"' + b[i:i + width].hex().upper() + '"')
    return "\n".join(lines)


def family(rid, name, draw):
    big, small = draw(32), draw(16)
    out = []
    for typ, data in (("ICN#", bits(big)), ("icl8", deep(big, C8, 8)), ("icl4", deep(big, C4, 4)),
                      ("ics#", bits(small)), ("ics8", deep(small, C8, 8)), ("ics4", deep(small, C4, 4))):
        out.append(f"data '{typ}' ({rid}, \"{name}\") {{\n{hexdata(data)}\n}};\n")
    return "".join(out)


def preview(path):
    """the 8-bit icons, 4x, on a gray ground, for a look before they go to the Mac"""
    import zlib, struct
    tiles = [calculator(32), card(32), calculator(16), card(16)]
    S, PAD = 4, 8
    w = PAD + sum(len(t) * S + PAD for t in tiles)
    h = 32 * S + 2 * PAD
    px = [[(160, 160, 160)] * w for _ in range(h)]
    x0 = PAD
    for t in tiles:
        for y, row in enumerate(t):
            for x, v in enumerate(row):
                if v is None:
                    continue
                col = rgb_of_cube(C8[v])
                for dy in range(S):
                    for dx in range(S):
                        px[PAD + y * S + dy][x0 + x * S + dx] = col
        x0 += len(t) * S + PAD
    raw = b"".join(b"\0" + bytes(sum(r, ())) for r in px)
    chunk = lambda t, d: struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)
    open(path, "wb").write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                           + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))


HEAD = """/* Bundle.r - Mac-59's Finder bundle and icons; written by mkbundle.py, do not edit by hand.
   Rez -a Bundle.r -o \"Project Resources.rsrc\", then build; after Build Application: SetFile -c TI59 -a Bi Mac-59
   (THINK writes the creator from Set Project Type; the bundle bit tells the Finder to read the BNDL). */

data 'TI59' (0, "Owner resource") {
	$"064D 6163 2D35 39"                                    /* "Mac-59" */
};

data 'BNDL' (128) {
	$"5449 3539 0000 0001"                                  /* owner 'TI59' 0, two types follow */
	$"4943 4E23 0001 0000 0080 0001 0081"                   /* 'ICN#': local 0 -> 128, 1 -> 129 */
	$"4652 4546 0002 0000 0080 0001 0081 0002 0082"         /* 'FREF': local 0 -> 128, 1 -> 129, 2 -> 130 */
};

data 'FREF' (128) { $"4150 504C 0000 00" };               /* 'APPL', icon 0 */
data 'FREF' (129) { $"5449 4344 0001 00" };               /* 'TICD', icon 1: binary cards */
data 'FREF' (130) { $"5445 5854 0001 00" };               /* 'TEXT', icon 1: Calc-U-59 text cards */

"""

if __name__ == "__main__":
    out = HEAD + family(128, "Mac-59", calculator) + family(129, "Magnetkarte", card)
    open("Bundle.r", "w", encoding="utf-8").write(out)
    if len(sys.argv) > 1:
        preview(sys.argv[1])
    print("Bundle.r", len(out), "chars")
