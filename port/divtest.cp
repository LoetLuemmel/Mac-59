/* divtest.cp - host and Mac: key the slim Mondlandung in, play it to the crash and print, after every
   key, the step count and an FNV-1a hash over the PC path; then the display while it blinks. The first
   line where host and Mac differ is the key during which the 68K build of the core takes another path.
   Optional: divtest <from> <to> also prints PC per step for steps from..to (the narrowing pass). */
#include "TI59Core.h"
#include "divkeys.h"
#include <stdio.h>
#include <stdlib.h>

static u32 gHash, gSteps, gFrom, gTo;

static void StepOne(TI59Core* m)
{
    u16 pc;
    m->Step();
    pc = m->PC();
    gHash = (gHash ^ (pc & 0xFF)) * 16777619UL;
    gHash = (gHash ^ (pc >> 8)) * 16777619UL;
    gSteps++;
    if (gSteps >= gFrom && gSteps <= gTo) printf("  %lu %04X\n", (unsigned long) gSteps, (unsigned) pc);
}

static void StepFor(TI59Core* m, u32 n) { u32 i; for (i = 0; i < n; i++) StepOne(m); }

static void RunToScan(TI59Core* m)
{
    u32 i;
    for (i = 0; i < 4000000UL; i++) {
        StepOne(m);
        if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) return;
    }
}

static void Show(TI59Core* m)
{
    TI59Display d;
    short i;
    m->TakeDisplay(&d);
    StepFor(m, 1421);
    m->TakeDisplay(&d);
    for (i = 11; i >= 0; i--) {
        if (!d.litTicks[i]) { putchar('_'); continue; }
        if (d.ctrl[i] == 5 || d.ctrl[i] == 6) putchar('-');
        else putchar('0' + d.digits[i] % 10);
    }
    putchar('\n');
}

int DivMain(const char* romPath, u32 from, u32 to)
{
    static u8 blob[kTI59BlobBytes];
    TI59Core* m;
    FILE* f;
    short k, n;
    gFrom = from; gTo = to; gHash = 2166136261UL; gSteps = 0;
    f = fopen(romPath, "rb");
    if (!f) return 1;
    fread(blob, 1, sizeof blob, f);
    fclose(f);
    m = new TI59Core;
    m->LoadBlob(blob);
    m->PowerOn();
    RunToScan(m);
    n = (short) (sizeof kDivKeys / sizeof kDivKeys[0]);
    for (k = 0; k < n; k++) {
        m->PressMatrix(kDivKeys[k]);
        StepFor(m, 1421);
        m->ReleaseMatrix(kDivKeys[k]);
        StepFor(m, 1421);
        RunToScan(m);
        printf("%d %d %lu %08lX\n", k, kDivKeys[k], (unsigned long) gSteps, (unsigned long) gHash);
    }
    for (k = 0; k < 6; k++) { StepFor(m, 20000); Show(m); }
    return 0;
}

#ifndef DIVTEST_NO_MAIN
int main(int argc, char** argv)
{
    return DivMain(argc > 1 ? argv[1] : "build/ti59rom.bin",
                   argc > 3 ? (u32) atol(argv[2]) : 0, argc > 3 ? (u32) atol(argv[3]) : 0);
}
#endif
