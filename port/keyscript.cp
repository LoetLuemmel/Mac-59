/* keyscript.cp - host only: run a key script on TI59Core (optionally with a module image) and print the display
   after each key, plus any printed lines. Keys are names or matrix codes:
     build/keyscript [-m module.bin] 2nd Pgm 1 4 E 5 A ...
   Names: 0-9 . +/- = + - x / ( ) A-E 2nd INV lnx CE CLR LRN x<>t x2 sqrt 1/x SST STO RCL SUM y^x BST EE GTO SBR RST R/S */
#include "TI59Core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static TI59Core* m;
static void Drain(void)
{
    static const char k[65] = " 0123456789ABCDE-FGHIJKLMNOPQRST.UVWXYZ+x*VPe(),^%~/='xx2?:!\"^PS";
    TI59PrintLine pl; char line[21]; short i;
    while (m->TakePrintLine(&pl)) {
        for (i = 0; i < 20; i++) line[i] = k[pl.codes[i] & 63];
        line[20] = 0;
        printf("       | %s\n", line);
    }
}
static void Step1(u32 i) { m->Step(); if ((i & 1023) == 0) Drain(); }   /* the printer queue holds 16 lines */
static void StepFor(u32 n) { u32 i; for (i = 0; i < n; i++) Step1(i); }
static void RunToScan(void)
{
    u32 i;
    for (i = 0; i < 30000000UL; i++) { Step1(i); if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) return; }
}
static void Show(const char* key)
{
    TI59Display d; short i, n = 0; char s[40];
    m->TakeDisplay(&d); StepFor(1421); m->TakeDisplay(&d);
    for (i = 11; i >= 0; i--) {
        if (!d.litTicks[i]) { if (n) s[n++] = ' '; continue; }
        switch (d.ctrl[i]) {
        case 0: case 1: case 8: case 9: s[n++] = (char) ('0' + d.digits[i] % 10); break;
        case 5: case 6: s[n++] = '-'; break;
        default: s[n++] = '?'; break;
        }
        if (d.dpTicks[i]) s[n++] = '.';
    }
    s[n] = 0;
    Drain();
    printf("%-6s %s%s\n", key, s, d.cTicks ? "   (C)" : "");
}
static const struct { const char* n; short c; } kNames[] = {
    {"A",11},{"B",12},{"C",13},{"D",14},{"E",15},{"2nd",21},{"INV",22},{"lnx",23},{"CE",24},{"CLR",25},
    {"LRN",31},{"x<>t",32},{"x2",33},{"sqrt",34},{"1/x",35},{"SST",41},{"STO",42},{"RCL",43},{"SUM",44},{"y^x",45},
    {"BST",51},{"EE",52},{"(",53},{")",54},{"/",55},{"GTO",61},{"7",62},{"8",63},{"9",64},{"x",65},
    {"SBR",71},{"4",72},{"5",73},{"6",74},{"-",75},{"RST",81},{"1",82},{"2",83},{"3",84},{"+",85},
    {"R/S",91},{"0",92},{".",93},{"+/-",94},{"=",95},{"Pgm",31},{0,0}};

int main(int argc, char** argv)
{
    static u8 blob[kTI59BlobBytes]; static u8 mod[5000];
    short a = 1, i, c;
    FILE* f = fopen("build/ti59rom.bin", "rb"); fread(blob, 1, sizeof blob, f); fclose(f);
    m = new TI59Core; m->LoadBlob(blob);
    if (argc > 2 && strcmp(argv[1], "-m") == 0) {
        f = fopen(argv[2], "rb"); fread(mod, 1, sizeof mod, f); fclose(f); m->LoadLibrary(mod, 5000); a = 3;
    }
    m->PowerOn(); m->SetPrinterConnected(1); RunToScan();
    for (; a < argc; a++) {
        for (c = 0, i = 0; kNames[i].n; i++) if (strcmp(kNames[i].n, argv[a]) == 0) c = kNames[i].c;
        if (!c) c = (short) atoi(argv[a]);
        m->PressMatrix(c); StepFor(1421); m->ReleaseMatrix(c); StepFor(1421); RunToScan();
        Show(argv[a]);
    }
    return 0;
}
