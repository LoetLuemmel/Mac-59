/* liststoptest.cp - host only: ListStop.h against TI59Core, with the app's key handling (StepTap) in
   emulated real time (237 weighted cycles per tick, printer drained each tick, R/S held 0.6 s while busy).
     build/liststoptest        exit 0 = every case as expected
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */

#include "Course.h"
#include "ListStop.h"
#include <stdio.h>
#include <string.h>

static TI59Core* m;
static ListStop ls;
static short q[256], qh, qc, tapKey, tapPhase;
static unsigned long tapCycles, tapHold;
static short printed[1000], nprinted, dropped;

static short Scan() { return (short) (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi); }

static void StepTap(unsigned long w)
{
    if (tapKey == 0) {
        if (ls.stopWanted) {                            /* as the app: R/S only into a busy TI */
            ls.stopWanted = 0;
            if (!Scan()) { tapKey = 91; tapHold = 8532; m->PressMatrix(91); tapPhase = 1; tapCycles = 0; }
            return;
        }
        if (qc == 0) return;
        tapKey = q[qh]; qh = (short) ((qh + 1) % 256); qc--;
        tapHold = Scan() ? 1422 : 8532;
        m->PressMatrix(tapKey); tapPhase = 1; tapCycles = 0;
        return;
    }
    tapCycles += w;
    if (tapPhase == 1) { if (tapCycles >= tapHold) { m->ReleaseMatrix(tapKey); tapPhase = 2; tapCycles = 0; } }
    else if (tapCycles >= 1422) { if (Scan() || tapCycles >= 5688) tapKey = 0; }
}

static void Tick(short stop)
{
    long budget = 237;
    TI59PrintLine pl;
    while (budget > 0) { unsigned long w = m->Step() & 0xFF; budget -= (long) w; StepTap(w); }
    while (m->TakePrintLine(&pl)) {
        short a = ListLineAddress(&pl), show = stop ? ListStopLine(&ls, &pl, m) : 1;
        if (!show) { dropped++; continue; }
        if (a >= 0 && nprinted < 1000) printed[nprinted++] = a;
    }
}

static void Keys(const char* s)
{
    short c[200], n = CourseKeys(s, c, 200), i;
    for (i = 0; i < n; i++) { q[(qh + qc) % 256] = c[i]; qc++; }
}

/* run until the keys are done and the TI idles (or seconds pass) */
static void Settle(double seconds, short stop)
{
    long t, idle = 0;
    for (t = 0; t < (long) (seconds * 60); t++) {
        Tick(stop);
        if (qc == 0 && tapKey == 0 && !ls.stopWanted && Scan()) { if (++idle > 120) return; } else idle = 0;
    }
}

static short fails, traceOn;

static void Case(const char* name, const char* setup, const char* list, short wantFirst, short wantLast,
                 short stop)
{
    char disp[40];
    TI59Display d;
    m->PowerOn(); Settle(3, 0);
    ListStopReset(&ls);
    Keys(setup); Settle(20, 0);
    nprinted = 0; dropped = 0;
    m->SetPrinterTrace((u8) traceOn);
    Keys(list); Settle(240, stop);
    m->TakeDisplay(&d); Tick(0); m->TakeDisplay(&d); CourseDisplayText(&d, disp);
    short ok = (short) (nprinted > 0 && printed[0] == wantFirst && printed[nprinted - 1] == wantLast && Scan());
    for (short i = 1; i < nprinted; i++) if (printed[i] != printed[i - 1] + 1) ok = 0;
    printf("%-4s %-42s lines %03d-%03d (%d, %d dropped), want %03d-%03d, idle %d, display \"%s\"\n",
           ok ? "ok" : "FAIL", name, nprinted ? printed[0] : -1, nprinted ? printed[nprinted - 1] : -1,
           nprinted, dropped, wantFirst, wantLast, Scan(), disp);
    if (!ok) fails++;
}

int main()
{
    static u8 blob[kTI59BlobBytes];
    FILE* f = fopen("build/ti59rom.bin", "rb");
    if (!f) { fprintf(stderr, "build/ti59rom.bin missing\n"); return 2; }
    fread(blob, 1, sizeof blob, f); fclose(f);
    m = new TI59Core; m->LoadBlob(blob); m->SetPrinterConnected(1);

    Case("x2 + 1 = R/S, stop on",       "LRN x2 + 1 = R/S LRN RST", "2nd =", 0, 7, 1);
    Case("x2 + 1 = R/S, stop off",      "LRN x2 + 1 = R/S LRN RST", "2nd =", 0, 479, 0);
    Case("ends in STO 00",              "LRN 4 STO 0 0 LRN RST", "2nd =", 0, 4, 1);
    Case("ends in Dsz 0 000 (97 00 00 00)", "LRN 1 2nd 0 0 0 0 0 LRN RST", "2nd =", 0, 4, 1);
    Case("empty memory",                "CLR", "2nd =", 0, 2, 1);
    Case("listing behind the program",  "LRN x2 + 1 LRN GTO 1 0 0", "2nd =", 100, 479, 1);
    Case("step at 477: partition ends", "GTO 4 7 7 LRN 5 LRN GTO 4 7 0", "2nd =", 470, 479, 1);
    Case("3 Op 17 (720 steps)",         "3 2nd 9 1 7 CLR LRN x2 + 1 = R/S LRN RST", "2nd =", 0, 7, 1);
    Case("step at 474: last stop that fits", "GTO 4 7 4 LRN 5 LRN GTO 4 7 0", "2nd =", 470, 477, 1);
    traceOn = 1;
    Case("TRACE on: run, then a listing", "LRN x2 + 1 = R/S LRN RST 3 R/S", "RST 2nd =", 0, 7, 1);
    traceOn = 0;
    printf("%s\n", fails ? "NICHT BESTANDEN" : "BESTANDEN");
    return fails ? 1 : 0;
}
