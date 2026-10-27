/* Course.cp - lessons of the programming course and their checks against TI59Core; see Course.h.
   Where the TI-59 keeps what a lesson checks (found by probing the core, port/memprobe in the notes):
     data register Rn       RAM register 119 - n; mantissa digits in nibbles 15..3 (15 = leading digit),
                            exponent in nibbles 2 (tens) and 1, nibble 0: bit 1 mantissa negative,
                            bit 2 exponent negative
     program step n         RAM register n / 8, byte n % 8 (low nibble first); the key code in BCD
     program pointer        SCOM register 0, nibbles 6..4 in BCD
     LRN mode               only in the display: "nnn cc"
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */

#include "Course.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const struct { const char* name; short code; } kCourseKeyNames[] = {
    {"A",11},{"B",12},{"C",13},{"D",14},{"E",15},{"2nd",21},{"INV",22},{"lnx",23},{"CE",24},{"CLR",25},
    {"LRN",31},{"x<>t",32},{"x2",33},{"sqrt",34},{"1/x",35},{"SST",41},{"STO",42},{"RCL",43},{"SUM",44},
    {"y^x",45},{"BST",51},{"EE",52},{"(",53},{")",54},{"/",55},{"GTO",61},{"7",62},{"8",63},{"9",64},
    {"x",65},{"SBR",71},{"4",72},{"5",73},{"6",74},{"-",75},{"RST",81},{"1",82},{"2",83},{"3",84},
    {"+",85},{"R/S",91},{"0",92},{".",93},{"+/-",94},{"=",95},{0,0}
};

short CourseKeys(const char* s, short* codes, short max)
{
    char w[16];
    short n = 0, i, k;
    while (*s) {
        while (*s == ' ' || *s == '\t') s++;
        if (!*s) break;
        for (i = 0; *s && *s != ' ' && *s != '\t' && i < 15; i++) w[i] = *s++;
        w[i] = 0;
        for (k = 0; kCourseKeyNames[k].name; k++) if (strcmp(kCourseKeyNames[k].name, w) == 0) break;
        if (kCourseKeyNames[k].name) { if (n < max) codes[n++] = kCourseKeyNames[k].code; }
        else if (w[0] >= '1' && w[0] <= '9' && w[1] >= '0' && w[1] <= '9' && !w[2]) {
            if (n < max) codes[n++] = (short) ((w[0] - '0') * 10 + w[1] - '0');
        }
        else return -1;
    }
    return n;
}

/* -- reading the calculator ---------------------------------------------------------------------- */

short CourseProgramCode(const TI59Core* m, short step)
{
    const u8* p = m->RamReg((short) (step / 8));
    short k = (short) (step % 8);
    return (short) (p[2 * k + 1] * 10 + p[2 * k]);
}

short CoursePointer(const TI59Core* m)
{
    return (short) (m->ScomNibble(0, 6) * 100 + m->ScomNibble(0, 5) * 10 + m->ScomNibble(0, 4));
}

double CourseRegister(const TI59Core* m, short reg)
{
    const u8* p = m->RamReg((short) (119 - reg));
    double mant = 0.0, scale = 1.0;
    short i, e;
    for (i = 15; i >= 3; i--) { mant += p[i] * scale; scale /= 10.0; }
    e = (short) (p[2] * 10 + p[1]);
    if (p[0] & 4) e = (short) -e;
    if (p[0] & 2) mant = -mant;
    while (e > 0) { mant *= 10.0; e--; }
    while (e < 0) { mant /= 10.0; e++; }
    return mant;
}

void CourseDisplayText(const TI59Display* d, char* out)
{
    char s[40];
    short i, n = 0, a, b;
    for (i = 11; i >= 0; i--) {
        if (!d->litTicks[i]) { s[n++] = ' '; continue; }
        switch (d->ctrl[i]) {
        case 0: case 1: case 8: case 9: s[n++] = (char) ('0' + d->digits[i] % 10); break;
        case 5: case 6: s[n++] = '-'; break;
        default: s[n++] = ' '; break;
        }
        if (d->dpTicks[i]) s[n++] = '.';
    }
    s[n] = 0;
    for (a = 0; s[a] == ' '; a++) ;
    for (b = n; b > a && s[b - 1] == ' '; b--) ;
    memcpy(out, s + a, (size_t) (b - a));
    out[b - a] = 0;
}

short CourseIsLRN(const char* d)
{
    short i;
    if (strlen(d) != 6 || d[3] != ' ') return 0;
    for (i = 0; i < 6; i++) if (i != 3 && (d[i] < '0' || d[i] > '9')) return 0;
    return 1;
}

short CourseDisplayValue(const char* d, double* v)
{
    double mant = 0.0, frac = 0.0;
    short neg = 0, any = 0, e = 0, eneg = 0, dot = 0;
    if (CourseIsLRN(d) || !*d) return 0;
    if (*d == '-') { neg = 1; d++; }
    for (; (*d >= '0' && *d <= '9') || *d == '.'; d++) {
        if (*d == '.') { dot = 1; frac = 0.1; continue; }
        any = 1;
        if (!dot) mant = mant * 10.0 + (*d - '0');
        else { mant += (*d - '0') * frac; frac /= 10.0; }
    }
    if (!any) return 0;
    while (*d == ' ') d++;
    if (*d) {                                           /* scientific: an exponent follows */
        if (*d == '-') { eneg = 1; d++; }
        for (; *d >= '0' && *d <= '9'; d++) e = (short) (e * 10 + (*d - '0'));
        if (eneg) e = (short) -e;
        while (e > 0) { mant *= 10.0; e--; }
        while (e < 0) { mant /= 10.0; e++; }
    }
    *v = neg ? -mant : mant;
    return 1;
}

static short Near(double a, double b)
{
    double d = a - b, m = (a < 0 ? -a : a) + (b < 0 ? -b : b);
    if (d < 0) d = -d;
    return d <= 1e-9 + 1e-7 * m;
}

short CourseHolds(const CourseCond* c, const TI59Core* m, const char* disp)
{
    double v;
    short i;
    switch (c->kind) {
    case kCondMode:     return (short) (CourseIsLRN(disp) == c->a);
    case kCondDisplay:  return (short) (strcmp(disp, c->text) == 0);
    case kCondValue:    return (short) (CourseDisplayValue(disp, &v) && Near(v, c->v));
    case kCondStep:     return (short) (CourseIsLRN(disp) && atoi(disp) == c->a);
    case kCondPointer:  return (short) (CoursePointer(m) == c->a);
    case kCondRegister: return Near(CourseRegister(m, c->a), c->v);
    case kCondProgram:
        for (i = 0; i < c->ncodes; i++) if (CourseProgramCode(m, (short) (c->a + i)) != c->codes[i]) return 0;
        return 1;
    }
    return 0;
}

static void Num(double v, char* out)
{
    char s[32];
    short n;
    sprintf(s, "%.8f", v);                              /* trailing zeros away */
    n = (short) strlen(s);
    while (n > 1 && s[n - 1] == '0') s[--n] = 0;
    if (n > 1 && s[n - 1] == '.') s[--n] = 0;
    strcpy(out, s);
}

void CourseDescribe(const CourseCond* c, const TI59Core* m, const char* disp, char* out)
{
    char a[32], b[32];
    short i;
    switch (c->kind) {
    case kCondMode:
        sprintf(out, "Modus %s, die Anzeige zeigt \"%s\"", c->a ? "LRN erwartet" : "Rechnen erwartet", disp);
        break;
    case kCondDisplay:  sprintf(out, "Anzeige \"%s\", erwartet \"%s\"", disp, c->text); break;
    case kCondValue:    Num(c->v, b); sprintf(out, "Anzeige \"%s\", erwartet %s", disp, b); break;
    case kCondStep:     sprintf(out, "Anzeige \"%s\", erwartet Schritt %03d", disp, c->a); break;
    case kCondPointer:  sprintf(out, "Programmzeiger %03d, erwartet %03d", CoursePointer(m), c->a); break;
    case kCondRegister:
        Num(CourseRegister(m, c->a), a); Num(c->v, b);
        sprintf(out, "Speicher %02d enthält %s, erwartet %s", c->a, a, b);
        break;
    case kCondProgram:
        for (i = 0; i < c->ncodes; i++)
            if (CourseProgramCode(m, (short) (c->a + i)) != c->codes[i]) {
                sprintf(out, "Schritt %03d enthält %02d, erwartet %02d", c->a + i,
                        CourseProgramCode(m, (short) (c->a + i)), c->codes[i]);
                return;
            }
        sprintf(out, "Programm ab %03d stimmt", c->a);
        break;
    default: strcpy(out, "?");
    }
}

/* -- parsing -------------------------------------------------------------------------------------- */

static short ParseCond(const char* s, CourseCond* c)
{
    char w[16];
    short i;
    memset(c, 0, sizeof *c);
    while (*s == ' ') s++;
    for (i = 0; *s && *s != ' ' && i < 15; i++) w[i] = *s++;
    w[i] = 0;
    while (*s == ' ') s++;
    if (strcmp(w, "modus") == 0) {
        c->kind = kCondMode;
        if (strncmp(s, "LRN", 3) == 0) c->a = 1; else if (strncmp(s, "HALT", 4) == 0) c->a = 0; else return 0;
    }
    else if (strcmp(w, "anzeige") == 0) {
        const char* e;
        c->kind = kCondDisplay;
        if (*s != '"') return 0;
        e = strchr(s + 1, '"');
        if (!e || e - s - 1 >= (long) sizeof c->text) return 0;
        memcpy(c->text, s + 1, (size_t) (e - s - 1));
    }
    else if (strcmp(w, "wert") == 0)     { c->kind = kCondValue; c->v = atof(s); }
    else if (strcmp(w, "schritt") == 0)  { c->kind = kCondStep; c->a = (short) atoi(s); }
    else if (strcmp(w, "zeiger") == 0)   { c->kind = kCondPointer; c->a = (short) atoi(s); }
    else if (strcmp(w, "register") == 0) {
        c->kind = kCondRegister;
        c->a = (short) atoi(s);
        while (*s && *s != ' ') s++;
        c->v = atof(s);
    }
    else if (strcmp(w, "programm") == 0) {
        c->kind = kCondProgram;
        c->a = (short) atoi(s);
        while (*s && *s != ' ') s++;
        while (*s) {
            while (*s == ' ') s++;
            if (!*s) break;
            if (c->ncodes >= 32) return 0;
            c->codes[c->ncodes++] = (short) atoi(s);
            while (*s && *s != ' ') s++;
        }
    }
    else return 0;
    return 1;
}

static short StartsWith(const char* line, const char* kw, const char** rest)
{
    size_t n = strlen(kw);
    if (strncmp(line, kw, n) != 0 || (line[n] != ' ' && line[n] != 0)) return 0;
    *rest = line + n;
    while (**rest == ' ') (*rest)++;
    return 1;
}

static void Copy(char* dst, const char* src, size_t max)
{
    strncpy(dst, src, max - 1);
    dst[max - 1] = 0;
}

short CourseParse(const char* buf, long n, CourseLesson* L, char* err)
{
    char line[1024];
    long pos = 0;
    short lineNo = 0, len, inText = 0, inProbe = 0, k;
    const char* r;
    CourseStep* st = 0;
    char part[16];

    memset(L, 0, sizeof *L);
    part[0] = 0;
    while (pos < n) {
        len = 0;
        while (pos < n && buf[pos] != '\r' && buf[pos] != '\n') {
            if (len >= (short) sizeof line - 1) { sprintf(err, "Zeile %d: zu lang", lineNo + 1); return 0; }
            line[len++] = buf[pos];
            pos++;
        }
        if (pos < n && buf[pos] == '\r' && pos + 1 < n && buf[pos + 1] == '\n') pos++;
        pos++;
        line[len] = 0;
        lineNo++;
        while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t')) line[--len] = 0;
        r = line;
        while (*r == ' ' || *r == '\t') r++;
        if (*r == '#') continue;

        if (StartsWith(r, "LEKTION", &r)) {
            L->number = (short) atoi(r);
            while (*r && *r != ' ') r++;
            while (*r == ' ') r++;
            Copy(L->title, r, sizeof L->title);
            inText = inProbe = 0;
        }
        else if (StartsWith(r, "ZIEL", &r)) { Copy(L->goal, r, sizeof L->goal); inText = inProbe = 0; }
        else if (StartsWith(r, "TEXT", &r)) { inText = 1; inProbe = 0; }
        else if (StartsWith(r, "TEIL", &r)) { Copy(part, r, sizeof part); inText = inProbe = 0; }
        else if (StartsWith(r, "SCHRITT", &r) || StartsWith(r, "AUFGABE", &r)) {
            if (L->nstep >= kCourseMaxSteps) { sprintf(err, "Zeile %d: zu viele Schritte", lineNo); return 0; }
            st = &L->step[L->nstep++];
            st->task = (short) (strncmp(line + strspn(line, " \t"), "AUFGABE", 7) == 0);
            Copy(st->part, part, sizeof st->part);
            Copy(st->say, r, sizeof st->say);
            inText = inProbe = 0;
        }
        else if (StartsWith(r, "PROBE", &r)) { inProbe = 1; inText = 0; }
        else if (StartsWith(r, "VORBEREITUNG", &r) || StartsWith(r, "LOESUNG", &r)) {
            short* keys;
            short* count;
            if (!st) { sprintf(err, "Zeile %d: ohne SCHRITT", lineNo); return 0; }
            if (strstr(line, "VORBEREITUNG")) { keys = st->prep; count = &st->nprep; }
            else { keys = st->solution; count = &st->nsolution; }
            *count = CourseKeys(r, keys, kCourseKeys);
            if (*count < 0) { sprintf(err, "Zeile %d: unbekannte Taste oder zu viele Tasten", lineNo); return 0; }
        }
        else if (StartsWith(r, "ERWARTE", &r)) {
            if (!st || st->nexpect >= kCourseMaxConds || !ParseCond(r, &st->expect[st->nexpect])) {
                sprintf(err, "Zeile %d: ERWARTE nicht lesbar", lineNo); return 0;
            }
            st->nexpect++;
        }
        else if (StartsWith(r, "TIPP", &r)) {
            if (!st || st->nhint >= kCourseMaxHints) { sprintf(err, "Zeile %d: TIPP", lineNo); return 0; }
            Copy(st->hint[st->nhint++], r, kCourseLine);
        }
        else if (StartsWith(r, "FEHLER", &r)) {
            char cond[kCourseLine];
            const char* colon = strstr(r, " : ");
            if (!st || st->nerror >= kCourseMaxErrors || !colon) { sprintf(err, "Zeile %d: FEHLER", lineNo); return 0; }
            memcpy(cond, r, (size_t) (colon - r));
            cond[colon - r] = 0;
            if (!ParseCond(cond, &st->error[st->nerror])) { sprintf(err, "Zeile %d: FEHLER-Bedingung", lineNo); return 0; }
            Copy(st->errorSay[st->nerror], colon + 3, kCourseLine);
            st->nerror++;
        }
        else if (StartsWith(r, "FALSCH", &r)) {
            if (!st || st->nerror == 0) { sprintf(err, "Zeile %d: FALSCH ohne FEHLER", lineNo); return 0; }
            k = (short) (st->nerror - 1);
            st->nwrong[k] = CourseKeys(r, st->wrong[k], kCourseKeys);
            if (st->nwrong[k] < 0) { sprintf(err, "Zeile %d: unbekannte Taste oder zu viele Tasten", lineNo); return 0; }
        }
        else if (inProbe && *r) {
            const char* arrow = strstr(r, "->");
            char keys[kCourseLine];
            CourseProbe* p;
            if (!arrow || L->nprobe >= kCourseMaxProbes) { sprintf(err, "Zeile %d: PROBE", lineNo); return 0; }
            p = &L->probe[L->nprobe++];
            memcpy(keys, r, (size_t) (arrow - r));
            keys[arrow - r] = 0;
            p->nkeys = CourseKeys(keys, p->keys, kCourseKeys);
            if (p->nkeys < 0) { sprintf(err, "Zeile %d: unbekannte Taste oder zu viele Tasten", lineNo); return 0; }
            p->want = atof(arrow + 2);
        }
        else if (inText) {
            size_t have = strlen(L->text);
            if (have + strlen(r) + 2 < sizeof L->text) {
                if (have) strcat(L->text, " ");
                strcat(L->text, r);
            }
        }
        else if (*r) { sprintf(err, "Zeile %d: unbekanntes Schlüsselwort", lineNo); return 0; }
    }
    if (L->nstep == 0) { strcpy(err, "keine Schritte"); return 0; }
    return 1;
}
