/* ListStop.h - an option of the app, not of the TI-59: stop 2nd List after the end of the program.

   The TI-59 lists "from the current position of the program pointer, to the end of program memory"
   (Personal Programming, VI-4) and leaves stopping to R/S; 00 is the digit 0, a valid step, so the
   firmware has no end of a program to stop at. The app guesses one: the last step that is not 00.
   It shows three lines more, since a program may end in up to three 00 steps of its own (STO 00,
   GTO 000, Dsz 0 000). It asks for R/S one line before the last of them, because the ROM sees R/S
   only between two lines and one more line comes, and drops whatever is printed after them.

   No stop when the listing starts behind that end (someone wants to see the empty memory) or when
   the partition ends within those three lines anyway: an R/S that came after the listing would start
   the program instead.

   A listing line: address in columns 6-8, key code in 11-12, columns 9-10 blank (measured; TRACE,
   Prt and INV List lines have another shape). Op 08's label lines have this shape, but their
   addresses lie inside the program and never reach the end.
   Header only, so the app (CMain.cp includes its sources) and port/liststoptest.cp share it.
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */

#ifndef LISTSTOP_H
#define LISTSTOP_H

#include "TI59Core.h"

enum { kListMargin = 3 };

struct ListStop {
    short inRun;        /* the previous printed line was a listing line */
    short addr;         /* its address */
    short last;         /* last step that is not 00, -1 for an empty memory; fixed when a run starts */
    short active;       /* this run will be stopped */
    short stopWanted;   /* the app should press R/S now (cleared by the app) */
    short stopAsked;
};

/* program steps of the partition: SCOM 9 nibble 0 counts the program's blocks of 80 steps
   (measured: 6 Op 17 -> 6, 480 steps; 3 Op 17 -> 9, 720 steps) */
static short ListProgramSteps(const TI59Core* m)
{
    short blocks = (short) (m->ScomNibble(9, 0) & 0x0F);
    if (blocks > 12) blocks = 12;
    return (short) (blocks * 80);
}

static short ListLastStep(const TI59Core* m)
{
    short n;
    for (n = (short) (ListProgramSteps(m) - 1); n >= 0; n--) {
        const u8* r = m->RamReg((short) (n / 8));
        short k = (short) (n % 8);
        if (r[2 * k] != 0 || r[2 * k + 1] != 0) return n;
    }
    return -1;
}

static void ListStopReset(ListStop* s)
{
    s->inRun = 0; s->addr = 0; s->last = -1; s->active = 0; s->stopWanted = 0; s->stopAsked = 0;
}

/* the line's address if it is a listing line, else -1; printer code 0 is a blank, 1..10 the digits */
static short ListLineAddress(const TI59PrintLine* pl)
{
    short i, a = 0;
    for (i = 6; i <= 12; i++) {
        u8 c = (u8) (pl->codes[i] & 0x3F);
        if (i == 9 || i == 10) { if (c != 0) return -1; }
        else if (c < 1 || c > 10) return -1;
    }
    for (i = 6; i <= 8; i++) a = (short) (a * 10 + (pl->codes[i] & 0x3F) - 1);
    return a;
}

/* one printed line; 0: drop it from the paper */
static short ListStopLine(ListStop* s, const TI59PrintLine* pl, const TI59Core* m)
{
    short a = ListLineAddress(pl);

    if (a < 0) { s->inRun = 0; return 1; }
    if (!s->inRun || a != s->addr + 1) {               /* a new listing */
        s->last = ListLastStep(m);
        s->active = (short) (a <= s->last + kListMargin
                             && s->last + kListMargin + 2 < ListProgramSteps(m));
        s->stopAsked = 0;
    }
    s->inRun = 1;
    s->addr = a;
    if (!s->active) return 1;
    if (a >= s->last + kListMargin - 1 && !s->stopAsked) { s->stopWanted = 1; s->stopAsked = 1; }   /* one line still comes */
    return (short) (a <= s->last + kListMargin);
}

#endif
