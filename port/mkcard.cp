/* mkcard.cp - host only: key a program in from a key script (LRN mode), let the TI-59 write bank 1 to a blank
   card and save that card in Calc-U-59's text format, then play a few keys to test it:
     build/mkcard programs/x.keys out.u59 "Title" [test keys ...]
   A key script holds key names (as keyscript.cp, plus "2nd" chords written as two names); '#' starts a comment.
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */
#include "TI59Core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static TI59Core* m;
static void StepFor(u32 n) { u32 i; TI59PrintLine pl; for (i = 0; i < n; i++) { m->Step(); if ((i & 1023) == 0) while (m->TakePrintLine(&pl)) {} } }
static void RunToScan(void)
{
    u32 i;
    for (i = 0; i < 60000000UL; i++) { m->Step(); if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) return; }
}
static const char* Disp(void)
{
    static char s[40]; TI59Display d; short i, n = 0;
    m->TakeDisplay(&d); StepFor(1421); m->TakeDisplay(&d);
    for (i = 11; i >= 0; i--) {
        if (!d.litTicks[i]) continue;
        switch (d.ctrl[i]) {
        case 0: case 1: case 8: case 9: s[n++] = (char) ('0' + d.digits[i] % 10); break;
        case 5: case 6: s[n++] = '-'; break;
        default: s[n++] = '?'; break;
        }
        if (d.dpTicks[i]) s[n++] = '.';
    }
    s[n] = 0;
    return s;
}
static const struct { const char* n; short c; } kNames[] = {
    {"A",11},{"B",12},{"C",13},{"D",14},{"E",15},{"2nd",21},{"INV",22},{"lnx",23},{"CE",24},{"CLR",25},
    {"LRN",31},{"x<>t",32},{"x2",33},{"sqrt",34},{"1/x",35},{"SST",41},{"STO",42},{"RCL",43},{"SUM",44},{"y^x",45},
    {"BST",51},{"EE",52},{"(",53},{")",54},{"/",55},{"GTO",61},{"7",62},{"8",63},{"9",64},{"x",65},
    {"SBR",71},{"4",72},{"5",73},{"6",74},{"-",75},{"RST",81},{"1",82},{"2",83},{"3",84},{"+",85},
    {"R/S",91},{"0",92},{".",93},{"+/-",94},{"=",95},{0,0}};
static short Code(const char* w)
{
    short i;
    for (i = 0; kNames[i].n; i++) if (strcmp(kNames[i].n, w) == 0) return kNames[i].c;
    fprintf(stderr, "unknown key %s\n", w); exit(1);
}
static void Tap(short c) { m->PressMatrix(c); StepFor(1421); m->ReleaseMatrix(c); StepFor(1421); RunToScan(); }
static void Run(void) { /* until the program stops (or 2 min of TI time) */
    u32 i; for (i = 0; i < 1706250UL * 60; i += 1421) { StepFor(1421); if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) { StepFor(20000); if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) return; } }
}

int main(int argc, char** argv)
{
    static u8 blob[kTI59BlobBytes]; static char text[20000]; char* w; short a, steps = 0;
    const u8* card; u32 i; FILE* f;
    if (argc < 4) { fprintf(stderr, "usage: mkcard keys out.u59 title [test keys]\n"); return 2; }
    f = fopen("build/ti59rom.bin", "rb"); fread(blob, 1, sizeof blob, f); fclose(f);
    m = new TI59Core; m->LoadBlob(blob); m->PowerOn(); RunToScan();
    f = fopen(argv[1], "r"); i = fread(text, 1, sizeof text - 1, f); text[i] = 0; fclose(f);
    Tap(25); Tap(31);                                         /* CLR LRN */
    for (w = strtok(text, " \t\n"); w; w = strtok(0, " \t\n")) {
        if (w[0] == '#') { strtok(0, "\n"); continue; }
        Tap(Code(w)); steps++;
    }
    fprintf(stderr, "LRN display at the end: %s\n", Disp());
    Tap(31);                                                  /* LRN off */
    fprintf(stderr, "%d keys keyed in\n", steps);
    Tap(25); Tap(82);                                         /* 1 2nd Write */
    m->PressMatrix(21); StepFor(1421); m->ReleaseMatrix(21); StepFor(1421); RunToScan();
    m->PressMatrix(91); StepFor(1421); m->ReleaseMatrix(91);
    for (i = 0; i < 200000 && !m->WaitingForCard(); i++) m->Step();
    m->InsertCard(0, 0);
    for (i = 0; i < 400000 && (m->CardPresent() || m->CardMode() != 0); i++) m->Step();
    card = m->CardEject(); RunToScan();
    if (!card) { fprintf(stderr, "no card written\n"); return 1; }
    f = fopen(argv[2], "w");
    fprintf(f, "Calc-U-59 Card 1.0\n# %s\n# written by the TI-59 core (port/mkcard.cp) from %s\n\nHEADER:\n", argv[3], argv[1]);
    fprintf(f, "Partition: %02X\nDataType: %s\nBank: %d\nProtection: %s\nChecksum: %02X\n\nDATA:\n",
            card[0], card[1] == 0x11 ? "program" : "data", (card[2] & 15) / 3 + 1, card[3] ? "yes" : "no", card[244]);
    for (a = 0; a < 30; a++)
        fprintf(f, "R%03d: %02X %02X %02X %02X %02X %02X %02X %02X\n", a, card[4 + a * 8], card[5 + a * 8], card[6 + a * 8],
                card[7 + a * 8], card[8 + a * 8], card[9 + a * 8], card[10 + a * 8], card[11 + a * 8]);
    fclose(f);
    printf("card %s written, display %s\n", argv[2], Disp());
    for (a = 4; a < argc; a++) {                              /* test: keys, "run" runs until the program stops */
        if (strcmp(argv[a], "run") == 0) { Run(); printf("  run    -> %s\n", Disp()); continue; }
        if (strcmp(argv[a], "scan") == 0) { short k; for (k = 0; k < 40; k++) { StepFor(20000UL); printf("  %2d     -> %s\n", k, Disp()); } continue; }
        if (strcmp(argv[a], "wait") == 0) { StepFor(3000000UL); printf("  wait   -> %s\n", Disp()); continue; }
        Tap(Code(argv[a]));
        printf("  %-6s -> %s\n", argv[a], Disp());
    }
    return 0;
}
