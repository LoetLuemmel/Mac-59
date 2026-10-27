# The demonstrations of the course lessons as (keys clicked, keys shown, explanation[, options]).
# Keys: face names (2nd INV lnx CE CLR LRN x<>t x2 sqrt 1/x SST STO RCL SUM y^x BST EE ( ) / GTO * SBR - RST
# + R/S . +/- =, A-E, digits), 2nd functions by their name (log CP Pgm Ins CMs Exc Prd Ind Del Fix Op Lbl x>=t
# Stflg Ifflg pi Write Dsz Prt List ...), printer buttons PRINT ADV TRACE, hold:<key> for a long press.
# Options: zoom [disp|disp4|prn|prnlow], delay (1/100 s), noshot, running (s, capture while busy), sleep.
D = {"zoom": ["disp", "disp4"]}      # short result: the display, then its digits
DL = {"zoom": ["disp"]}             # long result: the whole display

LESSONS = {
    "1-1-tasten": [
        ("", "", "Eingeschaltet: Die Anzeige zeigt 0."),
        ("100", "100", "Eine Zahl eintippen."),
        ("2nd", "2nd", "2nd: Die nächste Taste gilt mit ihrer oberen Beschriftung."),
        ("lnx", "2nd log", "log steht über lnx: der Zehnerlogarithmus von 100 ist 2.", D),
        ("CLR 2 INV log", "CLR 2 INV 2nd log", "INV kehrt die Funktion um: 10 hoch 2 = 100.", D),
        ("CLR 9 sqrt", "CLR 9 √x", "Die Quadratwurzel wirkt sofort auf die Anzeige: 3."),
        ("CLR 2 1/x", "CLR 2 1/x", "Der Kehrwert von 2: 0.5."),
        ("CLR pi", "CLR 2nd π", "π steht über der Taste 3 – mit zehn Stellen.", DL),
    ],
    "1-2-rechenregeln": [
        ("2 + 3", "2 + 3", "Der TI-59 merkt sich die Addition ..."),
        ("* 4", "× 4", "... denn ein × folgt: Punkt vor Strich."),
        ("=", "=", "Erst 3 × 4, dann + 2: 14.", D),
        ("( 2 + 3 ) * 4 =", "( 2 + 3 ) × 4 =", "Die Klammer zuerst: 5 × 4 = 20."),
        ("2 y^x 3 + 1 =", "2 yˣ 3 + 1 =", "Potenz vor Strich: 2³ = 8, dann + 1 = 9."),
        ("1 + 2 * 3 y^x 2", "1 + 2 × 3 yˣ 2", "Drei Ebenen warten: +, × und die Potenz ..."),
        ("=", "=", "... 3² = 9, × 2 = 18, + 1 = 19.", D),
    ],
    "1-3-speicher": [
        ("2 * 3 =", "2 × 3 =", "Ein Ergebnis: 6."),
        ("STO 01", "STO 01", "Ab in Speicher 01. Die Anzeige bleibt 6."),
        ("CLR", "CLR", "Anzeige gelöscht – der Speicher nicht."),
        ("RCL 01", "RCL 01", "Zurückgeholt: 6.", D),
        ("4 SUM 01", "4 SUM 01", "SUM addiert die Anzeige zum Speicher: 01 enthält jetzt 10."),
        ("RCL 01", "RCL 01", "Kontrolle: 10.", D),
        ("CMs RCL 01", "2nd CMs RCL 01", "2nd CMs löscht alle Speicher: 0."),
    ],
    "2-1-erstes-programm": [
        ("LRN", "LRN", "Lernmodus: Schritt 000, Inhalt 00.",DL),
        ("x2", "x²", "Die Taste wird gespeichert statt ausgeführt: weiter bei 001."),
        ("+ 1 = R/S", "+ 1 = R/S", "Fünf Schritte, der Zähler steht auf 005.",DL),
        ("LRN", "LRN", "Zurück im Rechenmodus."),
        ("RST", "RST", "Der Programmzeiger springt auf 000."),
        ("3 R/S", "3 R/S", "Das Programm rechnet 3² + 1 = 10.", D),
        ("RST List", "RST 2nd List", "Ein Listing auf dem Drucker ...", {"running": 2.8, "noshot": True}),
        ("R/S", "R/S", "Angehalten. Das Listing: Adresse, Code, Taste.", {"busy": True, "zoom": ["prn"]}),
    ],
    "2-2-eingabe-ausgabe": [
        ("CP LRN", "2nd CP LRN", "Programmspeicher löschen, Lernmodus."),
        ("- 32 = * 5 / 9 =", "− 3 2 = × 5 ÷ 9 =", "(x − 32) × 5 ÷ 9: das − wirkt auf die Startzahl."),
        ("Fix 2 R/S", "2nd Fix 2 R/S", "Zwei Nachkommastellen, dann halten: Schritt 012.",DL),
        ("LRN RST", "LRN RST", "Rechenmodus, Zeiger auf 000."),
        ("212 R/S", "212 R/S", "212 °F sind 100 °C.", D),
        ("RST 98.6 R/S", "RST 98.6 R/S", "98,6 °F sind 37 °C.", D),
        ("INV Fix", "INV 2nd Fix", "INV Fix schaltet die feste Stellenzahl wieder ab."),
    ],
    "2-3-funktionstasten": [
        ("CP LRN Lbl A", "2nd CP LRN 2nd Lbl A", "Ein Label am Anfang: Taste A startet hier.",DL),
        ("- 32 = * 5 / 9 = R/S", "− 3 2 = × 5 ÷ 9 = R/S", "Fahrenheit in Celsius."),
        ("Lbl B * 9 / 5 + 32 = R/S", "2nd Lbl B × 9 ÷ 5 + 3 2 = R/S", "Label B: Celsius in Fahrenheit. Schritt 023."),
        ("LRN", "LRN", "Fertig. Kein RST nötig: A und B suchen ihr Label selbst."),
        ("212 A Prt", "212 A 2nd Prt", "A: 212 °F = 100 °C; 2nd Prt druckt das Ergebnis.", D),
        ("100 B Prt", "100 B 2nd Prt", "B: 100 °C = 212 °F."),
        ("40 +/- A Prt", "40 +/- A 2nd Prt", "−40 ist in beiden Skalen gleich.", {"zoom": ["disp", "disp4", "prn"]}),
    ],
    "2-4-bearbeiten": [
        ("CP LRN * 3 + 3 = R/S LRN", "2nd CP LRN × 3 + 3 = R/S LRN", "Ein Programm mit Tippfehler: 3x + 3 statt 2x + 3."),
        ("RST LRN SST", "RST LRN SST", "SST: einen Schritt vor. Schritt 001 enthält 03.",DL),
        ("2", "2", "Überschrieben: jetzt 02, der Zeiger rückt auf 002."),
        ("LRN GTO 002 LRN", "LRN GTO 002 LRN", "Direkt zu Schritt 002: dort steht 85, das +.",DL),
        ("Ins", "2nd Ins", "Einfügen: 002 ist frei, das + rückt nach 003.",DL),
        ("x2 LRN", "x² LRN", "x² eingefügt. Das Programm rechnet jetzt x² × 2 + 3."),
        ("GTO 002 LRN Del", "GTO 002 LRN 2nd Del", "Löschen: x² ist weg, alles rückt wieder nach vorn.",DL),
        ("LRN RST 4 R/S", "LRN RST 4 R/S", "Kontrolle: 2 × 4 + 3 = 11.", D),
    ],
    "3-1-spruenge": [
        ("CP LRN * 2 GTO 008", "2nd CP LRN × 2 GTO 0 0 8", "Ein Sprung zur Adresse 008: GTO belegt drei Schritte."),
        ("* 10 = R/S", "× 1 0 = R/S", "× 10 auf 005–007 wird übersprungen, = steht auf 008."),
        ("LRN RST 5 R/S", "LRN RST 5 R/S", "5 × 2 = 10 – das × 10 lief nie.", D),
        ("RST List", "RST 2nd List", "Ein Listing auf dem Drucker ...", {"running": 4.2, "noshot": True}),
        ("R/S", "R/S", "Im Listing: 61 GTO, dann die Adresse 00 08.", {"busy": True, "zoom": ["prn"]}),
    ],
    "3-2-entscheiden": [
        ("CP LRN Lbl A STO 01 0 x<>t", "2nd CP LRN 2nd Lbl A STO 01 0 x<>t", "Zahl merken, t = 0 setzen."),
        ("RCL 01 x>=t B +/-", "RCL 01 2nd x≥t B +/-", "Ist x ≥ t, springe nach B – sonst Vorzeichen drehen."),
        ("Lbl B R/S LRN", "2nd Lbl B R/S LRN", "Label B: anhalten. Fertig."),
        ("5 +/- A", "5 +/- A", "Aus −5 wird 5.", D),
        ("7 A", "7 A", "7 bleibt 7."),
        ("RST List", "RST 2nd List", "Ein Listing auf dem Drucker ...", {"running": 5.5, "noshot": True}),
        ("R/S", "R/S", "x≥t heißt auf dem Drucker GE.", {"busy": True, "zoom": ["prn"]}),
    ],
    "3-3-schleifen": [
        ("CP LRN Lbl A STO 00 0 STO 01", "2nd CP LRN 2nd Lbl A STO 00 0 STO 01", "Zähler n in 00, Summe 0 in 01."),
        ("Lbl C RCL 00 SUM 01", "2nd Lbl C RCL 00 SUM 01", "Schleifenkörper: Zähler zur Summe."),
        ("Dsz 0 C RCL 01 R/S LRN", "2nd Dsz 0 C RCL 01 R/S LRN", "Dsz zählt 00 herunter und springt nach C, bis 0."),
        ("10 A Prt", "10 A 2nd Prt", "1 + 2 + … + 10 = 55.", D),
        ("100 A", "100 A", "100 Durchläufe: Solange das Programm läuft, zeigt der TI nur ein C.", {"running": 2.0}),
        ("Prt", "2nd Prt", "Nach einigen Sekunden: 5050.", {"zoom": ["disp", "disp4", "prn"]}),
    ],
    "3-4-flags": [
        ("CP LRN Lbl A Stflg 1 R/S", "2nd CP LRN 2nd Lbl A 2nd St flg 1 R/S", "A setzt Flag 1."),
        ("Lbl B INV Stflg 1 R/S", "2nd Lbl B INV 2nd St flg 1 R/S", "B löscht Flag 1."),
        ("Lbl C Ifflg 1 D 0 R/S", "2nd Lbl C 2nd If flg 1 D 0 R/S", "C: ist Flag 1 gesetzt, weiter bei D – sonst 0."),
        ("Lbl D 1 R/S LRN", "2nd Lbl D 1 R/S LRN", "D zeigt 1."),
        ("A C", "A C", "Flag gesetzt: 1.", D),
        ("B C", "B C", "Flag gelöscht: 0.", D),
    ],
    "4-1-unterprogramme": [
        ("CP LRN Lbl A STO 01 1 STO 02", "2nd CP LRN 2nd Lbl A STO 01 1 STO 02", "n in 01, Produkt 1 in 02."),
        ("Lbl C RCL 01 SBR D", "2nd Lbl C RCL 01 SBR D", "Den Zähler an das Unterprogramm D geben."),
        ("Dsz 1 C RCL 02 R/S", "2nd Dsz 1 C RCL 02 R/S", "Herunterzählen; am Ende das Produkt zeigen."),
        ("Lbl D Prd 02 INV SBR LRN", "2nd Lbl D 2nd Prd 02 INV SBR LRN", "D: 02 × Anzeige, INV SBR kehrt zurück."),
        ("5 A", "5 A", "5! = 120.", D),
        ("10 A Prt", "10 A 2nd Prt", "10! = 3628800.",DL),
        ("RST List", "RST 2nd List", "Ein Listing auf dem Drucker ...", {"running": 8.5, "noshot": True}),
        ("R/S", "R/S", "INV SBR steht als 92 RTN auf einem einzigen Schritt.", {"busy": True, "zoom": ["prnlow"]}),
    ],
    "4-2-indirekt": [
        ("CP LRN Lbl A STO Ind 00", "2nd CP LRN 2nd Lbl A STO 2nd Ind 00", "STO Ind 00: speichern in den Speicher, dessen Nummer in 00 steht.",DL),
        ("1 SUM 00 R/S LRN", "1 SUM 00 R/S LRN", "Den Zeiger in 00 um 1 erhöhen."),
        ("10 STO 00", "10 STO 00", "Der Zeiger zeigt auf Speicher 10."),
        ("7 A 8 A 9 A", "7 A 8 A 9 A", "Drei Eingaben ..."),
        ("RCL 10", "RCL 10", "... landen in 10, 11, 12: 7,", D),
        ("RCL 12", "RCL 12", "... und 9."),
        ("RCL 00", "RCL 00", "Der Zeiger steht auf dem nächsten freien Speicher: 13.", D),
    ],
    "4-3-partition-karten": [
        ("Op 16", "2nd Op 16", "Die Aufteilung: 479 Programmschritte, Speicher 0–59.", D),
        ("Prt 3 Op 17", "2nd Prt 3 2nd Op 17", "3 Op 17: 30 Speicher, dafür 719 Schritte.", D),
        ("Prt 6 Op 17 Prt", "2nd Prt 6 2nd Op 17 2nd Prt", "6 Op 17: zurück zur Grundeinstellung.", {"zoom": ["prn"]}),
    ],
    "4-4-module": [
        ("Pgm 14", "2nd Pgm 14", "Programm 14 der Leisure Library: Mars Lander."),
        ("E", "E", "Neuer Anflug: Geschwindigkeit und Höhe in einer Zahl.",DL),
        ("B", "B", "B: der Resttreibstoff."),
        ("C", "C", "C: noch einmal Geschwindigkeit und Höhe."),
        ("Pgm 14 Op 09 RST List", "2nd Pgm 14 2nd Op 09 RST 2nd List", "Op 09 holt das Modulprogramm in den Speicher – jetzt lässt es sich listen.", {"running": 6.0}),
        ("hold:R/S", "R/S (lange)", "R/S beendet den Ausdruck zwischen zwei Zeilen.", {"zoom": ["prn"], "busy": True}),
    ],
    "6-1-tastencodes": [
        ("CP LRN STO Ind", "2nd CP LRN STO 2nd Ind", "Drei Tasten ..."),
        ("01", "0 1", "... aber der Zähler springt von 000 auf 002.",DL),
        ("BST BST", "BST BST", "Schritt 000 enthält 72: STO und Ind in einem Code.",DL),
        ("LRN", "LRN", "Zurück in den Rechenmodus."),
        ("RST List", "RST 2nd List", "Ein Listing auf dem Drucker ...", {"running": 2.0, "noshot": True}),
        ("R/S", "R/S", "Auf dem Drucker: 72 ST*.", {"busy": True, "zoom": ["prn"]}),
    ],
    "6-2-speicheraufbau": [
        ("CP LRN x2 + 1", "2nd CP LRN x² + 1", "Drei Programmschritte."),
        ("BST", "BST", "Schritt 002: 01, die Ziffer 1.",DL),
        ("BST", "BST", "Schritt 001: 85, das +."),
        ("BST", "BST", "Schritt 000: 33, x². Im Register stehen sie als …018533.",DL),
        ("LRN CLR 1.5 EE 3 +/-", "LRN CLR 1.5 EE 3 +/-", "Eine Zahl: Mantisse und Exponent mit Vorzeichen.",DL),
        ("=", "=", "13 Stellen Mantisse, 2 Stellen Exponent; = lässt die Darstellung, wie sie ist."),
        ("INV EE", "INV EE", "INV EE schaltet zurück: 0.0015.", D),
        ("Op 16", "2nd Op 16", "Die Grenze zwischen Programm und Speichern: 479.59.", D),
    ],
    "6-3-takt-tastatur-drucker": [
        ("CP LRN x2 + 1 = R/S LRN RST", "2nd CP LRN x² + 1 = R/S LRN RST", "Das Programm aus Lektion 2.1."),
        ("TRACE", "TRACE", "TRACE am Drucker: jeder Schritt wird mitgedruckt."),
        ("3 R/S", "3 R/S", "Der Ablauf von 3² + 1 auf dem Papier.", {"zoom": ["prn"]}),
        ("TRACE CLR RST List", "TRACE CLR RST 2nd List", "Ein Listing läuft …", {"running": 4.0}),
        ("hold:R/S", "R/S (lange)", "… und hält nur bei langem Druck: geprüft wird zwischen zwei Zeilen.", {"zoom": ["prnlow"], "busy": True}),
    ],
}


def program(name):
    """a programs/*.keys file of the repo as clickable tokens (x is the multiplication key there)"""
    import os, re
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "programs", name)
    toks = []
    for line in open(path, encoding="utf-8"):
        line = line.split("#")[0]
        toks += ["*" if t == "x" else t for t in line.split()]
    return " ".join(toks)


def landing(short):
    d = {"delay": 150} if short else {}
    blink = {"timeout": 3, "lit": True, "zoom": ["disp", "disp4"], "zdelay": 320}
    return [
        ("LRN A", "LRN A", "A: neues Spiel. 100 ft/s abwärts, 2500 ft hoch, 100 Gallonen im Tank.", DL),
        ("0 R/S", "0 R/S", "Zug 1: nichts brennen. Vier Sekunden freier Fall.", d),
        ("0 R/S", "0 R/S", "Zug 2: noch einmal nichts. Die Fähre wird schneller.", d),
        ("0 R/S", "0 R/S", "Zug 3: weiter im freien Fall.", d),
        ("16 R/S", "16 R/S", "Zug 4: 16 Gallonen pro Sekunde bremsen kräftig.", d),
        ("30 R/S", "30 R/S", "Zug 5: 30 Gallonen pro Sekunde – volle Bremsung.", d),
        ("6 R/S", "6 R/S", "Zug 6: sanft nachregeln.", d),
        ("9 R/S", "9 R/S", "Aufgesetzt: 0.0000 blinkt – die perfekte Landung.", blink),
    ]


LESSONS["5-1-mondlandung"] = [
    ("CP LRN", "2nd CP LRN", "Lernmodus: Die Mondlandung wird Schritt für Schritt eingetippt.", DL),
    (program("mondlandung.keys"), "RCL 03 INV 2nd Fix 2nd Pause …", "Nach der letzten Taste steht der Zähler auf 126: Schritte 000 bis 125.", DL),
] + landing(False)

LESSONS["5-2-verdichten"] = [
    ("CP LRN", "2nd CP LRN", "Die schlanke Fassung, wieder im Lernmodus eingetippt."),
    (program("mondlandung_kurz.keys"), "RCL 03 INV 2nd Fix 2nd Pause …", "101 Schritte statt 126.", DL),
] + landing(True)
