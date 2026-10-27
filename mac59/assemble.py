#!/usr/bin/env python3
"""assemble.py - builds mac59/CMain.cp from parts/ (head, Rechner's colours/helpers/panes, LED, tail)."""
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
r = lambda n: open("parts/" + n).read()
p = r("panes.txt")
old = """	SetRect(&r, 300, 12, 492, 538);					/* the printer strip			*/
	RGBForeColor(&kPaper);
	PaintRect(&r);

"""
p = p.replace(old, "")
out = r("head.txt") + r("colours.txt") + "\n" + r("helpers1.txt") + "\n" + r("lighten.txt") + "\n\n" + r("led.txt") + p + "\n" + r("tail.txt")
out.encode("mac_roman")                     # MacRoman only: the bridge writes MacRoman (ö is fine, box drawing is not)
open("CMain.cp", "w").write(out)
print("CMain.cp", len(out.splitlines()), "lines")
