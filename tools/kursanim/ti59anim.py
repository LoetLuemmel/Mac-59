#!/usr/bin/env python3
"""ti59anim.py LESSON... - play a course lesson on the real TI-59 app in the local guest (Basilisk II + AppleBridge),
capture a frame after each step and build an animated GIF with a caption strip and display zooms.

Frames are guest framebuffer crops (fb_export), clicks are the host's real mouse (cliclick), so the
operator must keep their hands off the mouse while it runs."""
import os, subprocess, sys, time, json, shutil

HOST = os.environ.get("APPLEBRIDGE_HOST", os.path.expanduser("~/AppleBridge/host"))   # AppleBridge's host/ folder
sys.path.insert(0, HOST)
import fb_export                                   # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")
APP = os.environ.get("MAC59_APP", "Macintosh HD:Mac-59:Mac-59")             # the app on the guest
WIN = (0, 21, 520, 580)                            # TI-59 + printer, guest pixels
DISP = (16, 68, 272, 38)                           # the display glass, guest pixels
CANVAS_W, IMG_H, CAP_H = 560, 580, 74
FONT = "/System/Library/Fonts/Supplemental/Arial.ttf"
FONT_B = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
MONO = "/System/Library/Fonts/Supplemental/Courier New Bold.ttf"

FACE = {}
for r, row in enumerate([["A", "B", "C", "D", "E"], ["2nd", "INV", "lnx", "CE", "CLR"],
                         ["LRN", "x<>t", "x2", "sqrt", "1/x"], ["SST", "STO", "RCL", "SUM", "y^x"],
                         ["BST", "EE", "(", ")", "/"], ["GTO", "7", "8", "9", "*"],
                         ["SBR", "4", "5", "6", "-"], ["RST", "1", "2", "3", "+"],
                         ["R/S", "0", ".", "+/-", "="]]):
    for c, k in enumerate(row):
        FACE[k] = (42 + 56 * c, 187 + 44 * r)
SECOND = {"A'": "A", "B'": "B", "C'": "C", "D'": "D", "E'": "E", "log": "lnx", "CP": "CE",
          "Pgm": "LRN", "P->R": "x<>t", "sin": "x2", "cos": "sqrt", "tan": "1/x",
          "Ins": "SST", "CMs": "STO", "Exc": "RCL", "Prd": "SUM", "Ind": "y^x",
          "Del": "BST", "Eng": "EE", "Fix": "(", "Int": ")", "|x|": "/",
          "Pause": "GTO", "x=t": "7", "Nop": "8", "Op": "9", "Deg": "*",
          "Lbl": "SBR", "x>=t": "4", "S+": "5", "xbar": "6", "Rad": "-",
          "Stflg": "RST", "Ifflg": "1", "D.MS": "2", "pi": "3", "Grad": "+",
          "Write": "R/S", "Dsz": "0", "Adv": ".", "Prt": "+/-", "List": "="}
PRINTER = {"PRINT": (342, 69), "ADV": (411, 69), "TRACE": (480, 69)}


def sh(*a, **k):
    return subprocess.run(list(a), capture_output=True, text=True, **k)


def bridge(cmd):
    r = sh("/usr/bin/python3", os.path.join(HOST, "send_command.py"), cmd)
    return r.stdout


def origin():
    r = sh(os.path.join(HOST, "guest_input.py"), "--keep-front", "geometry")
    for line in r.stdout.splitlines():
        if line.startswith("mapping:"):          # host = (569 + gx, 119 + gy)
            p = line.split("(")[1].split(")")[0].replace("gx", "").replace("gy", "").split(",")
            return int(p[0].strip().rstrip("+").strip()), int(p[1].strip().rstrip("+").strip())
        if line.startswith("front app:") and "BasiliskII" not in line:
            raise SystemExit("BasiliskII is not front: " + line)
    raise SystemExit("no geometry: " + r.stdout + r.stderr)


def expand(tokens):
    """key names -> guest points; a number is clicked digit by digit, a 2nd function as 2nd + key."""
    pts = []
    for t in tokens.split():
        if t.startswith("hold:"):                 # a long press (R/S that has to stop a listing)
            pts.append(("hold", FACE[t[5:]]))
        elif t in PRINTER:                       # the printer is its own window: front, button, TI front again
            pts += [("click", (410, 32)), ("click", PRINTER[t]), ("click", (150, 32))]
        elif t in FACE:
            pts.append(("click", FACE[t]))
        elif t in SECOND:
            pts.append(("click", FACE["2nd"]))
            pts.append(("click", FACE[SECOND[t]]))
        elif all(ch in "0123456789." for ch in t):
            pts += [("click", FACE[ch]) for ch in t]
        else:
            raise SystemExit("unknown key " + t)
    return pts


def press(tokens, gap=0.3):
    """synthetic clicks through the bridge (CLICK verb): the host's mouse stays the operator's.
    A long press needs nothing extra: the app holds a key 0.6 s itself while the TI is busy."""
    for kind, (gx, gy) in expand(tokens):
        if (gx, gy) != FACE["R/S"]:              # never type into a running program
            t0 = time.time()
            while running() and time.time() - t0 < 90:
                time.sleep(0.3)
        out = bridge("CLICK:%d:%d" % (gx, gy))
        if "Click" not in out:
            raise SystemExit("CLICK failed: " + out)
        time.sleep(gap)


def grab(region):
    png, _ = fb_export.capture_png(region=region)
    return png


def lit_columns():
    """display columns with a lit LED segment (guest x)"""
    pid, _ = fb_export.check()
    fr = fb_export.parse_dump(fb_export.request_dump(pid))
    pal, px, rb = fr["palette"], fr["pixels"], fr["row_bytes"]
    cols = set()
    for y in range(72, 104, 2):
        for x in range(18, 288):
            if pal[3 * px[y * rb + x]] > 150:
                cols.add(x)
    return cols


def running():
    """the TI shows a lone C at the left while a program runs"""
    cols = lit_columns()
    return bool(cols) and max(cols) < 70


def settle(timeout=40.0, still=3, period=0.35):
    """wait until the program stopped and the display (and the printer) stopped changing"""
    t0, last, same = time.time(), None, 0
    while time.time() - t0 < timeout:
        cur = grab(WIN)
        same = same + 1 if cur == last and not running() else 0
        if same >= still - 1:
            return True
        last = cur
        time.sleep(period)
    return False


def relaunch():
    bridge("KEY:113:12:256")                      # Cmd-Q
    time.sleep(3)
    bridge("LAUNCH:" + APP)
    time.sleep(6)
    settle(10)


def caption(path, keys, text):
    args = ["magick", "-size", "%dx%d" % (CANVAS_W, CAP_H), "xc:#1d1f22"]
    if keys:
        args += ["-font", MONO, "-pointsize", "17", "-fill", "#ffb347",
                 "-annotate", "+14+26", keys]
    args += ["-font", FONT, "-pointsize", "15", "-fill", "#e8e8e8",
             "-annotate", "+14+%d" % (55 if keys else 30), text, path]
    sh(*args)


def compose(win_png, cap_png, out, crop=None):
    """window (or a crop of it, scaled up) on the canvas, caption below"""
    if crop is None:
        geo = ["(", win_png, ")", "-gravity", "center", "-background", "#55585c", "-extent",
               "%dx%d" % (CANVAS_W, IMG_H)]
    else:
        x, y, w, h = crop
        filt = "point" if abs(CANVAS_W / w - round(CANVAS_W / w)) < 0.01 else "triangle"
        geo = ["(", win_png, "-crop", "%dx%d+%d+%d" % (w, h, x, y), "+repage", "-filter", filt,
               "-resize", "%dx%d!" % (CANVAS_W, IMG_H), ")"]
    r = sh("magick", *geo, cap_png, "-append", "+repage", out)
    if r.returncode:
        raise SystemExit("magick: " + r.stderr)


def rect_for(cx, cy, w):
    h = w * IMG_H / CANVAS_W
    x = min(max(cx - w / 2, 0), 520 - w)
    y = min(max(cy - h / 2, 0), 580 - h)
    return (x, y, w, h)


def zoom_path(targets):
    """crop rectangles from the whole window through each target in turn; (rect, is_stop)"""
    cur, out = (0.0, 0.0, 520.0, 580.0), []
    for t in targets:
        nxt = rect_for(*ZOOMS[t])
        for f in (0.35, 0.7, 1.0):
            r = tuple(c + (n - c) * f for c, n in zip(cur, nxt))
            out.append((tuple(int(round(v)) for v in r), f == 1.0))
        cur = nxt
    return out


ZOOMS = {"disp": (152, 66, 280),                  # the display fills the width: 2x
         "disp4": (220, 66, 140),                 # right half of the display: 4x
         "prn": (412, 150, 210),                  # printer paper, top
         "prnlow": (412, 330, 210)}               # printer paper, further down


def lesson(name, steps):
    work = os.path.join(OUT, name)
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(work)
    relaunch()
    frames = []                                   # (file, delay in 1/100 s)
    n = 0
    for st in steps:
        keys, shown, text = st[0], st[1], st[2]
        opt = st[3] if len(st) > 3 else {}
        if not opt.get("busy"):
            settle(opt.get("timeout", 40.0))
        press(keys)
        if opt.get("sleep"):
            time.sleep(opt["sleep"])
        if opt.get("running"):                    # capture while the TI is still busy
            time.sleep(opt["running"])
        elif not settle(opt.get("timeout", 40.0)):
            print("  not settled after", repr(keys))
        if opt.get("lit"):                        # a blinking display: wait for the program's end, catch it lit
            t0 = time.time()
            while time.time() - t0 < 60:
                cols = lit_columns()
                if cols and max(cols) > 200 and not running():
                    break
                time.sleep(0.1)
        if opt.get("noshot"):
            continue
        n += 1
        win = os.path.join(work, "w%02d.png" % n)
        open(win, "wb").write(grab(WIN))
        cap = os.path.join(work, "c%02d.png" % n)
        caption(cap, shown, text)
        f = os.path.join(work, "f%02d.png" % n)
        compose(win, cap, f)
        frames.append((f, opt.get("delay", 230)))
        path = zoom_path(opt.get("zoom", []))
        for i, (rc, stop) in enumerate(path):
            zf = os.path.join(work, "f%02dz%02d.png" % (n, i))
            compose(win, cap, zf, rc)
            last = i == len(path) - 1
            frames.append((zf, opt.get("zdelay", 280) if last else (150 if stop else 8)))
        print("  frame", n, repr(shown))
    gif = os.path.join(OUT, name + ".gif")
    args = ["magick"]
    for f, d in frames:
        args += ["-delay", str(d), f]
    args += ["-loop", "0", "-layers", "OptimizePlus", gif]
    r = sh(*args)
    if r.returncode:
        raise SystemExit("gif: " + r.stderr)
    print(name, len(frames), "frames", os.path.getsize(gif) // 1024, "KB")


if __name__ == "__main__":
    from lessons import LESSONS
    names = sys.argv[1:] or list(LESSONS)
    for nm in names:
        print("==", nm)
        lesson(nm, LESSONS[nm])
