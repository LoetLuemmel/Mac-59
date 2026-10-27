#!/usr/bin/env python3
"""mkballoons.py - writes Ballonhilfe.r: Mac-59's Balloon Help (System 7 Help Manager), resources only, no code.
  'hmnu' 128/129/130  the menus Mac-59, Drucker, Modul (one message per item; enabled/disabled/checked)
  'hrct' 130          the calculator window (through 'hwin'): display, card strip, clock
  'hrct' 133 / 134    the 45 keys, item n = key n of kKeys: 133 the key's own function, 134 the 2nd function
                      (each only its own: the balloon switches, operator 2026-10-27) - CKeyPane::GetHelpResID takes 134 while 2nd is pending (the app
                      tracks it: 2nd pressed, next key not yet; operator, 2026-10-27). TCL places these balloons at the
                      key panes and shows a new one when the mouse enters a key or 2nd changes (CMain::QueueKey)
  'hrct' 131          the printer window: PRINT, ADV, TRACE, the paper - NOT through 'hwin': the paper's scrolling
                      moves that window's origin, so static rectangles slid off the keys (operator, 2026-10-27);
                      the panes take items 1-4 themselves (CKeyPane::SetHelp, CPaperText), TCL places the balloons
  'hrct' 132          empty: the manual windows ("Mac-59 - ..."), which would otherwise match "Mac-59"
  'hwin' 130          window title -> hrct; the Help Manager compares the first n characters, so the longer
                      "Mac-59 <en dash>" comes before "Mac-59"
Geometry from parts/tail.txt (keys 48 x 40 at 14 + 56 col, 118 + 44 row) and parts/panes.txt; key codes as the course
measured them on the core (lesson 6.1: row and column, 2nd adds 5 to the column, E' is 10; digits are 00-09).
On System 6 the resources are simply not read. Rez -a Ballonhilfe.r -o "Project Resources.rsrc" (hmnu: also there -
our menus 128-130 live in Project Resources.rsrc / Visual Architect.rsrc, the hmnu are separate resources).
Strings in MacRoman as \\0xNN escapes (the other .r files do the same)."""

KEYS = [  # row, col, legend, text, 2nd label, 2nd text   (codes are computed)
    (0, 0, "A", "Anwendertaste: springt zum Label A (2nd Lbl A) und startet dort.", "A'", "Anwendertaste A' (Label 2nd Lbl A')."),
    (0, 1, "B", "Anwendertaste: springt zum Label B und startet dort.", "B'", "Anwendertaste B'."),
    (0, 2, "C", "Anwendertaste: springt zum Label C und startet dort.", "C'", "Anwendertaste C'."),
    (0, 3, "D", "Anwendertaste: springt zum Label D und startet dort.", "D'", "Anwendertaste D'."),
    (0, 4, "E", "Anwendertaste: springt zum Label E und startet dort.", "E'", "Anwendertaste E'."),
    (1, 0, "2nd", "Die nächste Taste gilt mit ihrer Zweitfunktion (die Beschriftung über der Taste).", None, None),
    (1, 1, "INV", "Kehrt die folgende Funktion um: INV lnx = e hoch x, INV SUM zieht ab, INV Fix hebt Fix auf.", None, None),
    (1, 2, "lnx", "Natürlicher Logarithmus.", "log", "Zehnerlogarithmus; INV log = 10 hoch x."),
    (1, 3, "CE", "Löscht nur die zuletzt getippte Zahl.", "CP", "Löscht das Programm (alle Schritte auf 00)."),
    (1, 4, "CLR", "Löscht Anzeige und laufende Rechnung, beendet das Blinken.", None, None),
    (2, 0, "LRN", "Lernmodus ein/aus: Tasten werden Programmschritte; die Anzeige zeigt Schritt und Code.", "Pgm", "Wählt ein Modulprogramm: 2nd Pgm 14 = Mars Lander."),
    (2, 1, "x<>t", "Tauscht Anzeige und t-Register, den Vergleichswert für x=t und x>=t.", "P->R", "Polar- in Rechtwinkelkoordinaten (INV: umgekehrt)."),
    (2, 2, "x2", "Quadriert die Anzeige.", "sin", "Sinus (Winkelmaß: Deg, Rad, Grad)."),
    (2, 3, "√x", "Quadratwurzel.", "cos", "Kosinus."),
    (2, 4, "1/x", "Kehrwert.", "tan", "Tangens."),
    (3, 0, "SST", "Im Lernmodus einen Schritt vor; sonst einen Programmschritt ausführen.", "Ins", "Fügt im Lernmodus einen leeren Schritt ein."),
    (3, 1, "STO", "STO nn speichert die Anzeige in Speicher nn.", "CMs", "Löscht alle Speicher."),
    (3, 2, "RCL", "RCL nn holt Speicher nn in die Anzeige.", "Exc", "Exc nn tauscht Anzeige und Speicher nn."),
    (3, 3, "SUM", "SUM nn addiert die Anzeige zu Speicher nn; INV SUM zieht ab.", "Prd", "Prd nn multipliziert Speicher nn mit der Anzeige; INV Prd teilt."),
    (3, 4, "y^x", "Potenz: y y^x x =; INV y^x zieht die x-te Wurzel.", "Ind", "Indirekt: STO Ind 00 speichert dorthin, wohin Speicher 00 zeigt."),
    (4, 0, "BST", "Im Lernmodus einen Schritt zurück.", "Del", "Löscht im Lernmodus den angezeigten Schritt."),
    (4, 1, "EE", "Exponent eingeben (wissenschaftliche Schreibweise); INV EE beendet sie.", "Eng", "Technische Schreibweise: Exponent durch 3 teilbar."),
    (4, 2, "(", "Öffnet eine Klammer.", "Fix", "Fix n: feste Stellenzahl n; INV Fix hebt sie auf."),
    (4, 3, ")", "Schließt eine Klammer.", "Int", "Ganzzahliger Teil; INV Int: der Nachkommateil."),
    (4, 4, "÷", "Geteilt durch.", "|x|", "Betrag."),
    (5, 0, "GTO", "GTO nnn springt zu Schritt nnn, GTO Label zum Label; außerhalb eines Programms setzt GTO nnn den Zeiger.", "Pause", "Zeigt die Anzeige kurz; das Programm läuft weiter."),
    (5, 1, "7", "Ziffer 7.", "x=t", "Springt zum Label, wenn Anzeige = t (INV: wenn ungleich)."),
    (5, 2, "8", "Ziffer 8.", "Nop", "Ein Schritt, der nichts tut."),
    (5, 3, "9", "Ziffer 9.", "Op", "Op nn: Sonderfunktion nn, z. B. Op 16 zeigt, Op 17 ändert die Aufteilung."),
    (5, 4, "x", "Mal (die Maltaste).", "Deg", "Winkel in Grad."),
    (6, 0, "SBR", "SBR Label ruft ein Unterprogramm auf; INV SBR kehrt zurück.", "Lbl", "Setzt ein Label: 2nd Lbl A."),
    (6, 1, "4", "Ziffer 4.", "x>=t", "Springt zum Label, wenn Anzeige >= t (INV: wenn kleiner)."),
    (6, 2, "5", "Ziffer 5.", "∑+", "Statistik: Datenpunkt aufnehmen (INV: entfernen)."),
    (6, 3, "6", "Ziffer 6.", "x quer", "Mittelwert der Statistik."),
    (6, 4, "-", "Minus.", "Rad", "Winkel im Bogenmaß."),
    (7, 0, "RST", "Setzt den Programmzeiger auf Schritt 000.", "St flg", "St flg n setzt Flag n (INV: löscht es)."),
    (7, 1, "1", "Ziffer 1.", "If flg", "If flg n Label: springt, wenn Flag n gesetzt ist (INV: wenn nicht)."),
    (7, 2, "2", "Ziffer 2.", "D.MS", "Grad.MinutenSekunden in Dezimalgrad (INV: umgekehrt)."),
    (7, 3, "3", "Ziffer 3.", "π", "Die Kreiszahl 3.141592654."),
    (7, 4, "+", "Plus.", "Grad", "Winkel in Neugrad."),
    (8, 0, "R/S", "Startet oder hält ein Programm.", "Write", "Write n schreibt Bank n auf eine Karte: danach Mac-59 > Leere Karte einschieben..."),
    (8, 1, "0", "Ziffer 0.", "Dsz", "Dsz n Label: zählt Speicher n herunter und springt, solange er nicht 0 ist."),
    (8, 2, ".", "Dezimalpunkt.", "Adv", "Papiervorschub am Drucker."),
    (8, 3, "+/-", "Wechselt das Vorzeichen.", "Prt", "Druckt die Anzeige."),
    (8, 4, "=", "Ergebnis.", "List", "Druckt das Programm ab dem Zeiger; INV List: die Speicher."),
]
NOT_STORED = {"2nd", "LRN", "SST", "BST", "Ins", "Del"}

WINDOW = [  # top, left, bottom, right, text
    (6, 6, 70, 290, "Die Anzeige: zehn Stellen; im Lernmodus Schritt und Tastencode. Ein C links: ein Programm läuft. "
                    "Blinkt bei Fehler oder Spielende - CLR beendet das Blinken."),
    (74, 6, 110, 290, "Der Kartenstreifen: zeigt die eingelesene Karte und die Belegung von A bis E. "
                      "Ein Klick liest eine Karte ein, wie Mac-59 > Karte einlesen..."),
    (526, 6, 542, 150, "Der Takt: 455 kHz wie der TI-59, oder volle Geschwindigkeit (Mac-59 > Einstellungen...)."),
]
PRINTER = [
    (0, 0, 40, 69, "PRINT druckt die Anzeige, wie 2nd Prt."),
    (0, 69, 40, 138, "ADV schiebt das Papier eine Zeile vor."),
    (0, 138, 40, 208, "TRACE ein/aus: druckt bei jedem Programmschritt Taste und Ergebnis."),
    (40, 0, 548, 208, "Der Papierstreifen des PC-100C: markieren und kopieren. Drucker > Papier abreißen löscht ihn, "
                      "Papier sichern... speichert ihn als SimpleText-Dokument."),
]
SKIP = None
MENUS = {  # id: (title message, [items: (enabled, disabled, checked) or SKIP])
    128: ("Mac-59: Magnetkarten und Einstellungen.", [
        ("Liest eine Magnetkarte (.u59) ein, wie ein Doppelklick auf die Karte.", "Nicht verfügbar: das ROM fehlt.", ""),
        ("Schiebt eine leere Karte ein: vorher n 2nd Write tippen; danach fragt Mac-59 nach dem Namen.", "Nicht verfügbar: das ROM fehlt.", ""),
        ("Zeigt die Hilfe zur eingelesenen Karte: die Lektion des Programmierkurses oder die Mondlandung-Anleitung.",
         "Zur eingelesenen Karte gibt es keine eigene Hilfe.", ""),
        SKIP,
        ("Takt (455 kHz oder volle Geschwindigkeit) und Drucker.", "", ""),
    ]),
    129: ("Der Drucker PC-100C.", [
        ("Löscht den Papierstreifen.", "", ""),
        ("Speichert den Papierstreifen als SimpleText-Dokument.", "", ""),
        SKIP,
        ("Schließt den Drucker an.", "", "Der Drucker ist angeschlossen. Ohne Häkchen druckt der TI-59 nicht."),
        ("Hält 2nd List kurz nach dem Programmende an; der echte TI-59 listet bis zum Ende des Speichers.", "",
         "Eingeschaltet: 2nd List hält kurz nach dem Programmende an. Ausschalten, um wie am echten TI-59 alles zu listen."),
    ]),
    130: ("Das Modul Leisure Library und die Anleitungen.", [
        ("Wählt Programm 14 des Moduls (2nd Pgm 14) und beschriftet den Kartenstreifen; E startet das Spiel.",
         "Nicht verfügbar: das Modul fehlt (TMC0547.txt im Ordner ROM).", ""),
        ("Die Spielregeln von Mars Lander.", "", ""),
        SKIP,
        ("Die Mondlandung vom TI-57: Spielregeln und Geschichte.", "", ""),
        SKIP,
        ("Der Programmierkurs in 20 Lektionen, mit den Karten zum Kurs.", "", ""),
    ]),
}


def code(row, col, second):
    c = col + 1 + (5 if second else 0)
    return (row + 1) * 10 + (c % 10)


def esc(s):
    b = s.encode("mac_roman")
    if len(b) > 255:
        raise SystemExit(f"longer than 255 bytes: {s[:60]}...")
    return '"' + "".join(chr(x) if 32 <= x < 127 and chr(x) not in '"\\' else f"\\0x{x:02X}" for x in b) + '"'


def key_texts(row, col, legend, text, sec, stext):
    """(without 2nd, after 2nd) for one key"""
    digit = legend.isdigit()
    own = f"{legend}: {text}" + (" Nicht speicherbar." if legend in NOT_STORED else
                                 f" Code {legend.zfill(2) if digit else code(row, col, False)}.")
    if legend == "2nd":
        return own, "2nd ist aktiv: Die nächste Taste gilt mit ihrer Zweitfunktion (die Beschriftung über der Taste)."
    if not sec:
        return own, f"2nd {legend}: {legend} hat keine Zweitfunktion; die Taste gilt wie ohne 2nd."
    two = f"2nd {sec}: {stext}" + (" Nicht speicherbar." if sec in NOT_STORED else f" Code {code(row, col, True)}.")
    return own, two


def hrct(rid, name, items):
    out = [f"resource 'hrct' ({rid}, \"{name}\") {{", "\tHelpMgrVersion, hmDefaultOptions, 0, 0,", "\t{"]
    rows = []
    for top, left, bottom, right, text in items:
        tip = ((top + bottom) // 2, (left + right) // 2)
        rows.append(f"\t\tHMStringItem {{ {{{tip[0]}, {tip[1]}}}, {{{top}, {left}, {bottom}, {right}}},\n\t\t\t{esc(text)} }}")
    out.append(",\n".join(rows))
    out += ["\t}", "};", ""]
    return "\n".join(out)


def hmnu(mid, title, items):
    def item(x):
        if x is SKIP:
            return "\t\tHMSkipItem { }"
        e, d, c = x
        return f"\t\tHMStringItem {{ {esc(e)}, {esc(d)}, {esc(c)}, \"\" }}"
    rows = [f"\t\tHMStringItem {{ {esc(title)}, \"\", \"\", \"\" }}"] + [item(x) for x in items]
    return "\n".join([f"resource 'hmnu' ({mid}) {{", "\tHelpMgrVersion, hmDefaultOptions, 0, 0,",
                      "\tHMSkipItem { },", "\t{", ",\n".join(rows), "\t}", "};", ""])


def main():
    keys1, keys2 = [], []
    for row, col, legend, text, sec, stext in KEYS:
        top, left = 118 + 44 * row, 14 + 56 * col
        one, two = key_texts(row, col, legend, text, sec, stext)
        keys1.append((top, left, top + 40, left + 48, one))
        keys2.append((top, left, top + 40, left + 48, two))
    win = list(WINDOW)
    head = ("/* Ballonhilfe.r - Mac-59's Balloon Help; written by mkballoons.py, do not edit by hand.\n"
            "   Rez -a Ballonhilfe.r -o \"Project Resources.rsrc\" */\n\n#include \"Balloons.r\"\n\n")
    hwin = ("resource 'hwin' (130, \"Mac-59\") {\n\tHelpMgrVersion, hmDefaultOptions,\n\t{\n"
            "\t\t132, 'hrct', 8, \"Mac-59 \\0xD0\",\t\t/* the manual windows: no balloons */\n"
            "\t\t130, 'hrct', 6, \"Mac-59\"\n\t}\n};\n\n")
    empty = "resource 'hrct' (132, \"Anleitungen\") {\n\tHelpMgrVersion, hmDefaultOptions, 0, 0,\n\t{ }\n};\n\n"
    out = head + hwin + hrct(130, "Mac-59", win) + "\n" + hrct(131, "PC-100C", PRINTER) + "\n" + empty
    out += hrct(133, "Tasten", keys1) + "\n" + hrct(134, "Tasten nach 2nd", keys2) + "\n"
    out += "\n".join(hmnu(m, t, its) for m, (t, its) in MENUS.items())
    t, its = MENUS[130]                     # the Modul menu without a ROM: CMain maps it with HMSetMenuResID
    noRom = [(its[0][0], "Nicht verfügbar: das ROM fehlt (die fünf Dateien im Ordner ROM, siehe ROM fehlt).", "")] + its[1:]
    out += "\n" + hmnu(131, t, noRom)
    open("Ballonhilfe.r", "w", encoding="ascii").write(out)
    print("Ballonhilfe.r:", len(win), "window rects,", len(keys1), "keys x 2,", len(PRINTER), "printer rects,", len(MENUS), "menus")


if __name__ == "__main__":
    main()
