/* Menus.r - Rechner 2's menus, replacing Visual Architect's: Edit with "Select All" (cmdSelectAll = 23,
   TCL Commands.h) for the printer paper, the Rechner menu with the magnetic card commands (514, 515 in
   CMain.h) the Drucker menu (519-521, 525) and the Modul menu (522-524, 526); MBAR 1 adds both to the bar. TCL takes the command number from "#n" in the item text.
     Rez Menus.r -a -o "Project Resources.rsrc"   (a project file: THINK merges it into Rechner.\0xB9.rsrc and the app) */

#include "Types.r"

resource 'MENU' (3, "Edit", preload) {
	3,
	textMenuProc,
	0x7FFFFF80,
	enabled,
	"Edit",
	{
		"Undo#16", noIcon, "Z", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Cut#18", noIcon, "X", noMark, plain,
		"Copy#19", noIcon, "C", noMark, plain,
		"Paste#20", noIcon, "V", noMark, plain,
		"Clear#21", noIcon, noKey, noMark, plain,
		"Select All#23", noIcon, "A", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Show Clipboard#22", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (128, "Menu") {
	128,
	textMenuProc,
	allEnabled,
	enabled,
	"Mac-59",
	{
		"Karte einlesen\0xC9#514", noIcon, noKey, noMark, plain,
		"Leere Karte einschieben\0xC9#515", noIcon, noKey, noMark, plain,
		"Hilfe zur Karte#527", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Einstellungen\0xC9#512", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129, "Drucker") {
	129,
	textMenuProc,
	allEnabled,
	enabled,
	"Drucker",
	{
		"Papier abrei\0xA7en#519", noIcon, noKey, noMark, plain,
		"Papier sichern\0xC9#520", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Angeschlossen#521", noIcon, noKey, noMark, plain,
		"Listing am Programmende anhalten#525", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (130, "Modul") {
	130,
	textMenuProc,
	allEnabled,
	enabled,
	"Modul",
	{
		"Mars Lander (Leisure Library 14)#522", noIcon, noKey, noMark, plain,
		"Anleitung Mars Lander\0xC9#523", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Anleitung Mondlandung (TI-57)\0xC9#524", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Programmierkurs\0xC9#526", noIcon, noKey, noMark, plain
	}
};

resource 'MBAR' (1, preload) {
	{ 1, 2, 3, 128, 129, 130 }
};
