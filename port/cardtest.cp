/* cardtest.cp - the magnetic card path on TI59Core, host only: a program on a card and back.
   LRN x 2 = R/S LRN; 1 2nd Write with a blank card inserted when the ROM waits for one; eject;
   CLR 2nd CP; insert the card while idle (a card that arrives while a program runs is not read);
   RST 7 R/S -> 14.
   Also reports whether printing (printer connected) ever raises WaitingForCard. */

#include "TI59Core.h"
#include <stdio.h>
#include <string.h>

#define kTapSteps 1421UL
static TI59Core* m;
static u8 sawWait;

static void StepFor(u32 n) { u32 i; for (i = 0; i < n; i++) { m->Step(); if (m->WaitingForCard()) sawWait = 1; } }
static void RunToScan(void)
{
    u32 i;
    for (i = 0; i < 2000000UL; i++) {
        m->Step(); if (m->WaitingForCard()) sawWait = 1;
        if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) return;
    }
}
static void Tap(short c) { m->PressMatrix(c); StepFor(kTapSteps); m->ReleaseMatrix(c); StepFor(kTapSteps); RunToScan(); }
static void Keys(const short* k) { while (*k) Tap(*k++); }
static void Show(const char* what)
{
    TI59Display d; short i; char s[32]; short n = 0;
    m->TakeDisplay(&d); StepFor(kTapSteps); m->TakeDisplay(&d);
    for (i = 11; i >= 0; i--) {
        if (!d.litTicks[i]) continue;
        switch (d.ctrl[i]) {
        case 0: case 1: case 8: case 9: s[n++] = (char)('0' + d.digits[i] % 10); break;
        case 5: case 6: s[n++] = '-'; break;
        }
    }
    s[n] = 0;
    printf("%-28s display=%-12s wait=%d present=%d mode=%d\n", what, s, m->WaitingForCard(), m->CardPresent(), m->CardMode());
}

int main(int argc, char** argv)
{
    static u8 blob[kTI59BlobBytes];
    static const short prog[] = {25, 31, 65, 83, 95, 91, 31, 0};      /* CLR LRN x 2 = R/S LRN */
    static const short run7[] = {25, 81, 62, 91, 0};                   /* CLR RST 7 R/S */
    static const short cp[] = {25, 21, 24, 0};                         /* CLR 2nd CP */
    static const short prt[] = {83, 21, 94, 0};                        /* 2 2nd Prt */
    u8 card[246];
    const u8* out;
    u32 i;
    FILE* f = fopen(argc > 1 ? argv[1] : "build/ti59rom.bin", "rb");
    if (!f || fread(blob, 1, sizeof(blob), f) != sizeof(blob)) { printf("no rom\n"); return 1; }
    fclose(f);
    m = new TI59Core; m->LoadBlob(blob); m->PowerOn(); m->SetPrinterConnected(1);
    RunToScan();

    sawWait = 0; Keys(prt); StepFor(20000); Show("print 2 (printer on)");
    printf("  WaitingForCard during printing: %d\n", sawWait);

    Keys(prog); Keys(run7); Show("program, RST 7 R/S");

    sawWait = 0;
    m->PressMatrix(82); StepFor(kTapSteps); m->ReleaseMatrix(82); StepFor(kTapSteps); RunToScan();
    m->PressMatrix(21); StepFor(kTapSteps); m->ReleaseMatrix(21); StepFor(kTapSteps); RunToScan();
    m->PressMatrix(91); StepFor(kTapSteps); m->ReleaseMatrix(91);
    for (i = 0; i < 200000UL && !m->WaitingForCard(); i++) m->Step();
    printf("1 2nd Write: waiting after %lu steps\n", (unsigned long)i);
    m->InsertCard(0, 0);
    for (i = 0; i < 400000UL && (m->CardPresent() || m->CardMode() != 0); i++) m->Step();
    printf("  card passed after %lu steps\n", (unsigned long)i);
    out = m->CardEject();
    printf("  ejected: %s\n", out ? "bank data" : "NOTHING");
    if (!out) return 1;
    memcpy(card, out, 246);
    printf("  bytes 0..15:"); for (i = 0; i < 16; i++) printf(" %02x", card[i]); printf("\n");
    RunToScan(); Show("after write");

    Keys(cp); Show("CLR 2nd CP (no run)");

    m->InsertCard(card, 246);
    for (i = 0; i < 400000UL && (m->CardPresent() || m->CardMode() != 0); i++) m->Step();
    printf("read: card passed after %lu steps\n", (unsigned long)i);
    m->CardEject();
    RunToScan(); Show("after read");
    Keys(run7); Show("RST 7 R/S");
    return 0;
}
