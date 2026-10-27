/* Ballonhilfe.r - Mac-59's Balloon Help; written by mkballoons.py, do not edit by hand.
   Rez -a Ballonhilfe.r -o "Project Resources.rsrc" */

#include "Balloons.r"

resource 'hwin' (130, "Mac-59") {
	HelpMgrVersion, hmDefaultOptions,
	{
		132, 'hrct', 8, "Mac-59 \0xD0",		/* the manual windows: no balloons */
		130, 'hrct', 6, "Mac-59"
	}
};

resource 'hrct' (130, "Mac-59") {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{
		HMStringItem { {38, 148}, {6, 6, 70, 290},
			"Die Anzeige: zehn Stellen; im Lernmodus Schritt und Tastencode. Ein C links: ein Programm l\0x8Auft. Blinkt bei Fehler oder Spielende - CLR beendet das Blinken." },
		HMStringItem { {92, 148}, {74, 6, 110, 290},
			"Der Kartenstreifen: zeigt die eingelesene Karte und die Belegung von A bis E. Ein Klick liest eine Karte ein, wie Mac-59 > Karte einlesen..." },
		HMStringItem { {534, 78}, {526, 6, 542, 150},
			"Der Takt: 455 kHz wie der TI-59, oder volle Geschwindigkeit (Mac-59 > Einstellungen...)." }
	}
};

resource 'hrct' (131, "PC-100C") {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{
		HMStringItem { {20, 34}, {0, 0, 40, 69},
			"PRINT druckt die Anzeige, wie 2nd Prt." },
		HMStringItem { {20, 103}, {0, 69, 40, 138},
			"ADV schiebt das Papier eine Zeile vor." },
		HMStringItem { {20, 173}, {0, 138, 40, 208},
			"TRACE ein/aus: druckt bei jedem Programmschritt Taste und Ergebnis." },
		HMStringItem { {294, 104}, {40, 0, 548, 208},
			"Der Papierstreifen des PC-100C: markieren und kopieren. Drucker > Papier abrei\0xA7en l\0x9Ascht ihn, Papier sichern... speichert ihn als SimpleText-Dokument." }
	}
};

resource 'hrct' (132, "Anleitungen") {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{ }
};

resource 'hrct' (133, "Tasten") {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{
		HMStringItem { {138, 38}, {118, 14, 158, 62},
			"A: Anwendertaste: springt zum Label A (2nd Lbl A) und startet dort. Code 11." },
		HMStringItem { {138, 94}, {118, 70, 158, 118},
			"B: Anwendertaste: springt zum Label B und startet dort. Code 12." },
		HMStringItem { {138, 150}, {118, 126, 158, 174},
			"C: Anwendertaste: springt zum Label C und startet dort. Code 13." },
		HMStringItem { {138, 206}, {118, 182, 158, 230},
			"D: Anwendertaste: springt zum Label D und startet dort. Code 14." },
		HMStringItem { {138, 262}, {118, 238, 158, 286},
			"E: Anwendertaste: springt zum Label E und startet dort. Code 15." },
		HMStringItem { {182, 38}, {162, 14, 202, 62},
			"2nd: Die n\0x8Achste Taste gilt mit ihrer Zweitfunktion (die Beschriftung \0x9Fber der Taste). Nicht speicherbar." },
		HMStringItem { {182, 94}, {162, 70, 202, 118},
			"INV: Kehrt die folgende Funktion um: INV lnx = e hoch x, INV SUM zieht ab, INV Fix hebt Fix auf. Code 22." },
		HMStringItem { {182, 150}, {162, 126, 202, 174},
			"lnx: Nat\0x9Frlicher Logarithmus. Code 23." },
		HMStringItem { {182, 206}, {162, 182, 202, 230},
			"CE: L\0x9Ascht nur die zuletzt getippte Zahl. Code 24." },
		HMStringItem { {182, 262}, {162, 238, 202, 286},
			"CLR: L\0x9Ascht Anzeige und laufende Rechnung, beendet das Blinken. Code 25." },
		HMStringItem { {226, 38}, {206, 14, 246, 62},
			"LRN: Lernmodus ein/aus: Tasten werden Programmschritte; die Anzeige zeigt Schritt und Code. Nicht speicherbar." },
		HMStringItem { {226, 94}, {206, 70, 246, 118},
			"x<>t: Tauscht Anzeige und t-Register, den Vergleichswert f\0x9Fr x=t und x>=t. Code 32." },
		HMStringItem { {226, 150}, {206, 126, 246, 174},
			"x2: Quadriert die Anzeige. Code 33." },
		HMStringItem { {226, 206}, {206, 182, 246, 230},
			"\0xC3x: Quadratwurzel. Code 34." },
		HMStringItem { {226, 262}, {206, 238, 246, 286},
			"1/x: Kehrwert. Code 35." },
		HMStringItem { {270, 38}, {250, 14, 290, 62},
			"SST: Im Lernmodus einen Schritt vor; sonst einen Programmschritt ausf\0x9Fhren. Nicht speicherbar." },
		HMStringItem { {270, 94}, {250, 70, 290, 118},
			"STO: STO nn speichert die Anzeige in Speicher nn. Code 42." },
		HMStringItem { {270, 150}, {250, 126, 290, 174},
			"RCL: RCL nn holt Speicher nn in die Anzeige. Code 43." },
		HMStringItem { {270, 206}, {250, 182, 290, 230},
			"SUM: SUM nn addiert die Anzeige zu Speicher nn; INV SUM zieht ab. Code 44." },
		HMStringItem { {270, 262}, {250, 238, 290, 286},
			"y^x: Potenz: y y^x x =; INV y^x zieht die x-te Wurzel. Code 45." },
		HMStringItem { {314, 38}, {294, 14, 334, 62},
			"BST: Im Lernmodus einen Schritt zur\0x9Fck. Nicht speicherbar." },
		HMStringItem { {314, 94}, {294, 70, 334, 118},
			"EE: Exponent eingeben (wissenschaftliche Schreibweise); INV EE beendet sie. Code 52." },
		HMStringItem { {314, 150}, {294, 126, 334, 174},
			"(: \0x85ffnet eine Klammer. Code 53." },
		HMStringItem { {314, 206}, {294, 182, 334, 230},
			"): Schlie\0xA7t eine Klammer. Code 54." },
		HMStringItem { {314, 262}, {294, 238, 334, 286},
			"\0xD6: Geteilt durch. Code 55." },
		HMStringItem { {358, 38}, {338, 14, 378, 62},
			"GTO: GTO nnn springt zu Schritt nnn, GTO Label zum Label; au\0xA7erhalb eines Programms setzt GTO nnn den Zeiger. Code 61." },
		HMStringItem { {358, 94}, {338, 70, 378, 118},
			"7: Ziffer 7. Code 07." },
		HMStringItem { {358, 150}, {338, 126, 378, 174},
			"8: Ziffer 8. Code 08." },
		HMStringItem { {358, 206}, {338, 182, 378, 230},
			"9: Ziffer 9. Code 09." },
		HMStringItem { {358, 262}, {338, 238, 378, 286},
			"x: Mal (die Maltaste). Code 65." },
		HMStringItem { {402, 38}, {382, 14, 422, 62},
			"SBR: SBR Label ruft ein Unterprogramm auf; INV SBR kehrt zur\0x9Fck. Code 71." },
		HMStringItem { {402, 94}, {382, 70, 422, 118},
			"4: Ziffer 4. Code 04." },
		HMStringItem { {402, 150}, {382, 126, 422, 174},
			"5: Ziffer 5. Code 05." },
		HMStringItem { {402, 206}, {382, 182, 422, 230},
			"6: Ziffer 6. Code 06." },
		HMStringItem { {402, 262}, {382, 238, 422, 286},
			"-: Minus. Code 75." },
		HMStringItem { {446, 38}, {426, 14, 466, 62},
			"RST: Setzt den Programmzeiger auf Schritt 000. Code 81." },
		HMStringItem { {446, 94}, {426, 70, 466, 118},
			"1: Ziffer 1. Code 01." },
		HMStringItem { {446, 150}, {426, 126, 466, 174},
			"2: Ziffer 2. Code 02." },
		HMStringItem { {446, 206}, {426, 182, 466, 230},
			"3: Ziffer 3. Code 03." },
		HMStringItem { {446, 262}, {426, 238, 466, 286},
			"+: Plus. Code 85." },
		HMStringItem { {490, 38}, {470, 14, 510, 62},
			"R/S: Startet oder h\0x8Alt ein Programm. Code 91." },
		HMStringItem { {490, 94}, {470, 70, 510, 118},
			"0: Ziffer 0. Code 00." },
		HMStringItem { {490, 150}, {470, 126, 510, 174},
			".: Dezimalpunkt. Code 93." },
		HMStringItem { {490, 206}, {470, 182, 510, 230},
			"+/-: Wechselt das Vorzeichen. Code 94." },
		HMStringItem { {490, 262}, {470, 238, 510, 286},
			"=: Ergebnis. Code 95." }
	}
};

resource 'hrct' (134, "Tasten nach 2nd") {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{
		HMStringItem { {138, 38}, {118, 14, 158, 62},
			"2nd A': Anwendertaste A' (Label 2nd Lbl A'). Code 16." },
		HMStringItem { {138, 94}, {118, 70, 158, 118},
			"2nd B': Anwendertaste B'. Code 17." },
		HMStringItem { {138, 150}, {118, 126, 158, 174},
			"2nd C': Anwendertaste C'. Code 18." },
		HMStringItem { {138, 206}, {118, 182, 158, 230},
			"2nd D': Anwendertaste D'. Code 19." },
		HMStringItem { {138, 262}, {118, 238, 158, 286},
			"2nd E': Anwendertaste E'. Code 10." },
		HMStringItem { {182, 38}, {162, 14, 202, 62},
			"2nd ist aktiv: Die n\0x8Achste Taste gilt mit ihrer Zweitfunktion (die Beschriftung \0x9Fber der Taste)." },
		HMStringItem { {182, 94}, {162, 70, 202, 118},
			"2nd INV: INV hat keine Zweitfunktion; die Taste gilt wie ohne 2nd." },
		HMStringItem { {182, 150}, {162, 126, 202, 174},
			"2nd log: Zehnerlogarithmus; INV log = 10 hoch x. Code 28." },
		HMStringItem { {182, 206}, {162, 182, 202, 230},
			"2nd CP: L\0x9Ascht das Programm (alle Schritte auf 00). Code 29." },
		HMStringItem { {182, 262}, {162, 238, 202, 286},
			"2nd CLR: CLR hat keine Zweitfunktion; die Taste gilt wie ohne 2nd." },
		HMStringItem { {226, 38}, {206, 14, 246, 62},
			"2nd Pgm: W\0x8Ahlt ein Modulprogramm: 2nd Pgm 14 = Mars Lander. Code 36." },
		HMStringItem { {226, 94}, {206, 70, 246, 118},
			"2nd P->R: Polar- in Rechtwinkelkoordinaten (INV: umgekehrt). Code 37." },
		HMStringItem { {226, 150}, {206, 126, 246, 174},
			"2nd sin: Sinus (Winkelma\0xA7: Deg, Rad, Grad). Code 38." },
		HMStringItem { {226, 206}, {206, 182, 246, 230},
			"2nd cos: Kosinus. Code 39." },
		HMStringItem { {226, 262}, {206, 238, 246, 286},
			"2nd tan: Tangens. Code 30." },
		HMStringItem { {270, 38}, {250, 14, 290, 62},
			"2nd Ins: F\0x9Fgt im Lernmodus einen leeren Schritt ein. Nicht speicherbar." },
		HMStringItem { {270, 94}, {250, 70, 290, 118},
			"2nd CMs: L\0x9Ascht alle Speicher. Code 47." },
		HMStringItem { {270, 150}, {250, 126, 290, 174},
			"2nd Exc: Exc nn tauscht Anzeige und Speicher nn. Code 48." },
		HMStringItem { {270, 206}, {250, 182, 290, 230},
			"2nd Prd: Prd nn multipliziert Speicher nn mit der Anzeige; INV Prd teilt. Code 49." },
		HMStringItem { {270, 262}, {250, 238, 290, 286},
			"2nd Ind: Indirekt: STO Ind 00 speichert dorthin, wohin Speicher 00 zeigt. Code 40." },
		HMStringItem { {314, 38}, {294, 14, 334, 62},
			"2nd Del: L\0x9Ascht im Lernmodus den angezeigten Schritt. Nicht speicherbar." },
		HMStringItem { {314, 94}, {294, 70, 334, 118},
			"2nd Eng: Technische Schreibweise: Exponent durch 3 teilbar. Code 57." },
		HMStringItem { {314, 150}, {294, 126, 334, 174},
			"2nd Fix: Fix n: feste Stellenzahl n; INV Fix hebt sie auf. Code 58." },
		HMStringItem { {314, 206}, {294, 182, 334, 230},
			"2nd Int: Ganzzahliger Teil; INV Int: der Nachkommateil. Code 59." },
		HMStringItem { {314, 262}, {294, 238, 334, 286},
			"2nd |x|: Betrag. Code 50." },
		HMStringItem { {358, 38}, {338, 14, 378, 62},
			"2nd Pause: Zeigt die Anzeige kurz; das Programm l\0x8Auft weiter. Code 66." },
		HMStringItem { {358, 94}, {338, 70, 378, 118},
			"2nd x=t: Springt zum Label, wenn Anzeige = t (INV: wenn ungleich). Code 67." },
		HMStringItem { {358, 150}, {338, 126, 378, 174},
			"2nd Nop: Ein Schritt, der nichts tut. Code 68." },
		HMStringItem { {358, 206}, {338, 182, 378, 230},
			"2nd Op: Op nn: Sonderfunktion nn, z. B. Op 16 zeigt, Op 17 \0x8Andert die Aufteilung. Code 69." },
		HMStringItem { {358, 262}, {338, 238, 378, 286},
			"2nd Deg: Winkel in Grad. Code 60." },
		HMStringItem { {402, 38}, {382, 14, 422, 62},
			"2nd Lbl: Setzt ein Label: 2nd Lbl A. Code 76." },
		HMStringItem { {402, 94}, {382, 70, 422, 118},
			"2nd x>=t: Springt zum Label, wenn Anzeige >= t (INV: wenn kleiner). Code 77." },
		HMStringItem { {402, 150}, {382, 126, 422, 174},
			"2nd \0xB7+: Statistik: Datenpunkt aufnehmen (INV: entfernen). Code 78." },
		HMStringItem { {402, 206}, {382, 182, 422, 230},
			"2nd x quer: Mittelwert der Statistik. Code 79." },
		HMStringItem { {402, 262}, {382, 238, 422, 286},
			"2nd Rad: Winkel im Bogenma\0xA7. Code 70." },
		HMStringItem { {446, 38}, {426, 14, 466, 62},
			"2nd St flg: St flg n setzt Flag n (INV: l\0x9Ascht es). Code 86." },
		HMStringItem { {446, 94}, {426, 70, 466, 118},
			"2nd If flg: If flg n Label: springt, wenn Flag n gesetzt ist (INV: wenn nicht). Code 87." },
		HMStringItem { {446, 150}, {426, 126, 466, 174},
			"2nd D.MS: Grad.MinutenSekunden in Dezimalgrad (INV: umgekehrt). Code 88." },
		HMStringItem { {446, 206}, {426, 182, 466, 230},
			"2nd \0xB9: Die Kreiszahl 3.141592654. Code 89." },
		HMStringItem { {446, 262}, {426, 238, 466, 286},
			"2nd Grad: Winkel in Neugrad. Code 80." },
		HMStringItem { {490, 38}, {470, 14, 510, 62},
			"2nd Write: Write n schreibt Bank n auf eine Karte: danach Mac-59 > Leere Karte einschieben... Code 96." },
		HMStringItem { {490, 94}, {470, 70, 510, 118},
			"2nd Dsz: Dsz n Label: z\0x8Ahlt Speicher n herunter und springt, solange er nicht 0 ist. Code 97." },
		HMStringItem { {490, 150}, {470, 126, 510, 174},
			"2nd Adv: Papiervorschub am Drucker. Code 98." },
		HMStringItem { {490, 206}, {470, 182, 510, 230},
			"2nd Prt: Druckt die Anzeige. Code 99." },
		HMStringItem { {490, 262}, {470, 238, 510, 286},
			"2nd List: Druckt das Programm ab dem Zeiger; INV List: die Speicher. Code 90." }
	}
};

resource 'hmnu' (128) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	HMSkipItem { },
	{
		HMStringItem { "Mac-59: Magnetkarten und Einstellungen.", "", "", "" },
		HMStringItem { "Liest eine Magnetkarte (.u59) ein, wie ein Doppelklick auf die Karte.", "Nicht verf\0x9Fgbar: das ROM fehlt.", "", "" },
		HMStringItem { "Schiebt eine leere Karte ein: vorher n 2nd Write tippen; danach fragt Mac-59 nach dem Namen.", "Nicht verf\0x9Fgbar: das ROM fehlt.", "", "" },
		HMStringItem { "Zeigt die Hilfe zur eingelesenen Karte: die Lektion des Programmierkurses oder die Mondlandung-Anleitung.", "Zur eingelesenen Karte gibt es keine eigene Hilfe.", "", "" },
		HMSkipItem { },
		HMStringItem { "Takt (455 kHz oder volle Geschwindigkeit) und Drucker.", "", "", "" }
	}
};

resource 'hmnu' (129) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	HMSkipItem { },
	{
		HMStringItem { "Der Drucker PC-100C.", "", "", "" },
		HMStringItem { "L\0x9Ascht den Papierstreifen.", "", "", "" },
		HMStringItem { "Speichert den Papierstreifen als SimpleText-Dokument.", "", "", "" },
		HMSkipItem { },
		HMStringItem { "Schlie\0xA7t den Drucker an.", "", "Der Drucker ist angeschlossen. Ohne H\0x8Akchen druckt der TI-59 nicht.", "" },
		HMStringItem { "H\0x8Alt 2nd List kurz nach dem Programmende an; der echte TI-59 listet bis zum Ende des Speichers.", "", "Eingeschaltet: 2nd List h\0x8Alt kurz nach dem Programmende an. Ausschalten, um wie am echten TI-59 alles zu listen.", "" }
	}
};

resource 'hmnu' (130) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	HMSkipItem { },
	{
		HMStringItem { "Das Modul Leisure Library und die Anleitungen.", "", "", "" },
		HMStringItem { "W\0x8Ahlt Programm 14 des Moduls (2nd Pgm 14) und beschriftet den Kartenstreifen; E startet das Spiel.", "Nicht verf\0x9Fgbar: das Modul fehlt (TMC0547.txt im Ordner ROM).", "", "" },
		HMStringItem { "Die Spielregeln von Mars Lander.", "", "", "" },
		HMSkipItem { },
		HMStringItem { "Die Mondlandung vom TI-57: Spielregeln und Geschichte.", "", "", "" },
		HMSkipItem { },
		HMStringItem { "Der Programmierkurs in 20 Lektionen, mit den Karten zum Kurs.", "", "", "" }
	}
};

resource 'hmnu' (131) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	HMSkipItem { },
	{
		HMStringItem { "Das Modul Leisure Library und die Anleitungen.", "", "", "" },
		HMStringItem { "W\0x8Ahlt Programm 14 des Moduls (2nd Pgm 14) und beschriftet den Kartenstreifen; E startet das Spiel.", "Nicht verf\0x9Fgbar: das ROM fehlt (die f\0x9Fnf Dateien im Ordner ROM, siehe ROM fehlt).", "", "" },
		HMStringItem { "Die Spielregeln von Mars Lander.", "", "", "" },
		HMSkipItem { },
		HMStringItem { "Die Mondlandung vom TI-57: Spielregeln und Geschichte.", "", "", "" },
		HMSkipItem { },
		HMStringItem { "Der Programmierkurs in 20 Lektionen, mit den Karten zum Kurs.", "", "", "" }
	}
};
