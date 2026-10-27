/* coursecheck.cp - host only: play a lesson file against TI59Core and check that it can be passed.
     build/coursecheck course/lektion2.txt [-l]      (-l: print the program memory afterwards)
   For every step: VORBEREITUNG runs; the step must not be fulfilled already; every FALSCH key sequence
   (from the state before the step) must produce its FEHLER state and must not pass the step; the
   LOESUNG must fulfil every ERWARTE. Then each PROBE runs in a copy of the calculator. Exit 0 = passed.
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */

#include "Course.h"
#include <stdio.h>
#include <string.h>

static short Scan(const TI59Core* m) { return (short) (m->PC() >= kTI59ScanLoLo && m->PC() <= kTI59ScanLoHi); }
static void Run(TI59Core* m, u32 n) { u32 i; for (i = 0; i < n; i++) m->Step(); }
static void ToScan(TI59Core* m) { u32 i; for (i = 0; i < 60000000UL; i++) { m->Step(); if (Scan(m)) return; } }
static void Tap(TI59Core* m, short c) { m->PressMatrix(c); Run(m, 1421); m->ReleaseMatrix(c); Run(m, 1421); ToScan(m); }
static void Keys(TI59Core* m, const short* k, short n) { short i; for (i = 0; i < n; i++) Tap(m, k[i]); }
static void Disp(TI59Core* m, char* out)
{
    TI59Display d;
    m->TakeDisplay(&d);
    Run(m, 1421);
    m->TakeDisplay(&d);
    CourseDisplayText(&d, out);
}
static short AllHold(const CourseStep* s, TI59Core* m, const char* disp)
{
    short i;
    for (i = 0; i < s->nexpect; i++) if (!CourseHolds(&s->expect[i], m, disp)) return 0;
    return 1;
}

int main(int argc, char** argv)
{
    static u8 blob[kTI59BlobBytes];
    static char text[60000];
    static CourseLesson L;
    char err[120], disp[40], why[200];
    TI59Core *m, *c;
    FILE* f;
    long n;
    short i, j, fails = 0;
    double v;

    if (argc < 2) { fprintf(stderr, "usage: coursecheck lesson.txt\n"); return 2; }
    f = fopen("build/ti59rom.bin", "rb"); fread(blob, 1, sizeof blob, f); fclose(f);
    f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "cannot read %s\n", argv[1]); return 2; }
    n = (long) fread(text, 1, sizeof text, f);
    fclose(f);
    if (!CourseParse(text, n, &L, err)) { printf("%s: %s\n", argv[1], err); return 1; }
    printf("Lektion %d: %s  (%d Schritte, %d Proben)\n", L.number, L.title, L.nstep, L.nprobe);

    m = new TI59Core;
    m->LoadBlob(blob);
    m->PowerOn();
    ToScan(m);
    for (i = 0; i < L.nstep; i++) {
        const CourseStep* s = &L.step[i];
        short ok = 1;
        Keys(m, s->prep, s->nprep);
        Disp(m, disp);
        if (AllHold(s, m, disp)) { printf("  %2d FEHLT: schon vor dem Schritt erfüllt\n", i + 1); ok = 0; }
        for (j = 0; j < s->nerror; j++) {
            char d2[40];
            if (s->nwrong[j] == 0) { printf("  %2d WARNUNG: FEHLER %d ohne FALSCH-Tasten\n", i + 1, j + 1); continue; }
            c = new TI59Core(*m);
            Keys(c, s->wrong[j], s->nwrong[j]);
            Disp(c, d2);
            if (!CourseHolds(&s->error[j], c, d2)) {
                CourseDescribe(&s->error[j], c, d2, why);
                printf("  %2d FEHLT: FALSCH %d erzeugt den Fehler nicht (%s)\n", i + 1, j + 1, why);
                ok = 0;
            }
            if (AllHold(s, c, d2)) { printf("  %2d FEHLT: FALSCH %d besteht den Schritt\n", i + 1, j + 1); ok = 0; }
            delete c;
        }
        if (s->nsolution == 0) { printf("  %2d FEHLT: keine LOESUNG\n", i + 1); ok = 0; }
        Keys(m, s->solution, s->nsolution);
        Disp(m, disp);
        for (j = 0; j < s->nexpect; j++)
            if (!CourseHolds(&s->expect[j], m, disp)) {
                CourseDescribe(&s->expect[j], m, disp, why);
                printf("  %2d FEHLT: LOESUNG erfüllt ERWARTE %d nicht: %s\n", i + 1, j + 1, why);
                ok = 0;
            }
        printf("  %2d %-4s %-9s %s  [Anzeige \"%s\"]\n", i + 1, ok ? "ok" : "--", s->part, s->say, disp);
        if (!ok) fails++;
    }
    for (i = 0; i < L.nprobe; i++) {
        const CourseProbe* p = &L.probe[i];
        short ok;
        c = new TI59Core(*m);
        Keys(c, p->keys, p->nkeys);
        Disp(c, disp);
        ok = (short) (CourseDisplayValue(disp, &v) && (v - p->want < 1e-7 && p->want - v < 1e-7));
        printf("  P%d %-4s Anzeige \"%s\", erwartet %g\n", i + 1, ok ? "ok" : "--", disp, p->want);
        if (!ok) fails++;
        delete c;
    }
    if (argc > 2 && strcmp(argv[2], "-l") == 0) {        /* the program memory as the lesson left it */
        short last = -1;
        for (i = 0; i < 480; i++) if (CourseProgramCode(m, i) != 0) last = i;
        printf("Listing 000-%03d:", last);
        for (i = 0; i <= last; i++) printf("%s%03d %02d", i % 8 ? "  " : "\n  ", i, CourseProgramCode(m, i));
        printf("\n");
    }
    printf("%s\n", fails ? "NICHT BESTANDEN" : "BESTANDEN");
    return fails ? 1 : 0;
}
