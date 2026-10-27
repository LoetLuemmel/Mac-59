/* portrun.cp - the acceptance run of the port: the key sequences of host-ref/refrun.cpp on TI59Core,
   printed in the same format. Must reproduce host-ref/reference.txt line for line.

     port/build.sh && build/portrun build/ti59rom.bin | diff host-ref/reference.txt -

   Same subset of C++ as the core (stdio only), so it can run on the Mac as a self-test too. */

#include "TI59Core.h"
#include <stdio.h>

/* 0.1 s of emulated instructions: (u32)(14218.75 * 0.1) */
#define kTapSteps 1421UL

static u32 gHash, gSteps;

static void StepOne(TI59Core* m)
{
    u16 pc;
    m->Step();
    pc = m->PC();
    gHash = (gHash ^ (pc & 0xFF)) * 16777619UL;
    gHash = (gHash ^ (pc >> 8)) * 16777619UL;
    gSteps++;
}

static void StepFor(TI59Core* m, u32 n)
{
    u32 i;
    for (i = 0; i < n; i++) StepOne(m);
}

static short RunToScanLoop(TI59Core* m)
{
    u32 i;
    for (i = 0; i < 2000000UL; i++) {
        StepOne(m);
        if (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi) return 1;
    }
    return 0;
}

static void Tap(TI59Core* m, short code)
{
    m->PressMatrix(code);
    StepFor(m, kTapSteps);
    m->ReleaseMatrix(code);
    StepFor(m, kTapSteps);
    RunToScanLoop(m);
}

static void DisplayText(TI59Core* m, char* s)
{
    TI59Display d;
    short i;
    m->TakeDisplay(&d);
    for (i = 11; i >= 0; i--) {
        if (d.litTicks[i] == 0) continue;
        switch (d.ctrl[i]) {
        case 0: case 1: case 8: case 9: *s++ = (char)('0' + (d.digits[i] & 0xF) % 10); break;
        case 5: *s++ = d.digits[i] == 0 ? '-' : 'o'; break;
        case 6: *s++ = '-'; break;
        default: break;
        }
    }
    *s = 0;
}

/* matrix codes, row x 10 + column */
enum {
    k2nd = 21, kINV = 22, kLnx = 23, kCE = 24, kCLR = 25,
    kLRN = 31, kXT = 32, kSq = 33, kSqrt = 34, kRecip = 35,
    kSST = 41, kSTO = 42, kRCL = 43, kSUM = 44, kPow = 45,
    kEE = 52, kLP = 53, kRP = 54, kDiv = 55,
    kGTO = 61, k7 = 62, k8 = 63, k9 = 64, kMul = 65,
    k4 = 72, k5 = 73, k6 = 74, kSub = 75,
    kRST = 81, k1 = 82, k2 = 83, k3 = 84, kAdd = 85,
    kRS = 91, k0 = 92, kPt = 93, kNeg = 94, kEq = 95
};

struct Seq { const char* name; short keys[12]; };   /* keys end at 0 */

static const Seq kSeqs[] = {
    {"boot",         {0}},
    {"aos_2+3x4",    {kCLR, k2, kAdd, k3, kMul, k4, kEq, 0}},
    {"pow_2^10",     {kCLR, k2, kPow, k1, k0, kEq, 0}},
    {"sin30+pi",     {kCLR, k3, k0, k2nd, kSq, kAdd, k2nd, k3, kEq, 0}},
    {"sqrt2",        {kCLR, k2, kSqrt, 0}},
    {"paren",        {kCLR, kLP, k2, kAdd, k3, kRP, kMul, k4, kEq, 0}},
    {"recip0_error", {kCLR, k0, kRecip, 0}},
    {"sto_rcl",      {kCLR, k5, kSTO, k0, k1, kCLR, kRCL, k0, k1, 0}},
    {"program_x2",   {kCLR, kLRN, kMul, k2, kEq, kRS, kLRN, kRST, k7, kRS, 0}},
    {"ee_1.5e3",     {kCLR, k1, kPt, k5, kEE, k3, kEq, 0}},
    {"lnx_2",        {kCLR, k2, kLnx, 0}},
    {"inv_sin_0.5",  {kCLR, kPt, k5, kINV, k2nd, kSq, 0}},
    {"div_7/3",      {kCLR, k7, kDiv, k3, kEq, 0}},
    {"sub_neg",      {kCLR, k5, kSub, k8, kEq, 0}},
};

int main(int argc, char** argv)
{
    static u8 blob[kTI59BlobBytes];
    const char* path = argc > 1 ? argv[1] : "build/ti59rom.bin";
    FILE* f;
    TI59Core* m;
    short s, k, ok;
    char text[32];

    f = fopen(path, "rb");
    if (!f) { perror(path); return 1; }
    if (fread(blob, 1, sizeof(blob), f) != sizeof(blob)) { fprintf(stderr, "%s: short read\n", path); return 1; }
    fclose(f);

    m = new TI59Core;
    m->LoadBlob(blob);
    for (s = 0; s < (short)(sizeof(kSeqs) / sizeof(kSeqs[0])); s++) {
        gHash = 2166136261UL;
        gSteps = 0;
        m->PowerOn();
        m->SetPrinterConnected(0);
        ok = RunToScanLoop(m);
        for (k = 0; kSeqs[s].keys[k]; k++) Tap(m, kSeqs[s].keys[k]);
        /* read over a fresh window, as the reference does */
        { TI59Display d; m->TakeDisplay(&d); }
        StepFor(m, kTapSteps);
        DisplayText(m, text);
        printf("%-14s display=%-14s steps=%-8lu pcfnv=%08lx%s\n", kSeqs[s].name, text,
               (unsigned long)gSteps, (unsigned long)gHash, ok ? "" : " BOOT-FAILED");
    }
    delete m;
    return 0;
}
