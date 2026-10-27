/* TI59.r - resources of Rechner 2 (TI-59 from its ROM), merged into "Project Resources.rsrc" with
     Rez TI59.r -a -o "Project Resources.rsrc"
   run in the project folder, next to ti59rom.bin (host-ref/mkblob.cpp: 6144 ROM words big-endian +
   1024 constant nibbles; Texas Instruments' firmware, private use).
   DLOG/DITL 201: the settings - 1 OK, 2 Abbrechen, 3 full speed, 4 printer connected, 5 note. */

#define SystemSevenOrLater 1
#include "Types.r"

read 'TROM' (128, "TI-59 ROM", purgeable) "ti59rom.bin";

/* the Leisure Library solid-state module (TMC0547, host-ref/mkmodule.cpp: 5000 bytes) - TI firmware, private use */
read 'TLIB' (128, "Leisure Library", purgeable) "leisure.bin";

resource 'DLOG' (201, "Einstellungen") {
	{0, 0, 150, 340},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	201,
	"Einstellungen",
	centerMainScreen
};

resource 'DITL' (201, "Einstellungen") {
	{
		{118, 250, 138, 320}, Button { enabled, "OK" },
		{118, 160, 138, 240}, Button { enabled, "Abbrechen" },
		{16, 20, 34, 320}, CheckBox { enabled, "Volle Geschwindigkeit" },
		{40, 20, 58, 320}, CheckBox { enabled, "Drucker PC-100C angeschlossen" },
		{68, 20, 108, 320}, StaticText { disabled, "Ohne volle Geschwindigkeit l\0x8Auft Mac-59 im Takt des TI-59 (455 kHz)." }
	}
};
