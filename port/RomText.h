/* RomText.h - the TI-59's ROM and a solid-state module from Calc-U-59's text dumps (its roms/ folder), so the
   app need not carry Texas Instruments' firmware: the same reading as upstream/Shared/RomLoader.cpp
   (parseRomTxt, parseConstantsTxt, parseModuleTxt), in the core's C++ subset, on a buffer holding one file.
   Lines may end in CR, LF or CR LF (a file copied to the Mac may have either); tokens are split on spaces
   and tabs.
     ROM:        after the first "---" line, "AAAA: wwww wwww ..." (AAAA 4 hex digits), words & 0x1FFF;
                 TMC0582.txt + TMC0583.txt + TMC0571B.txt in this order = 6144 words, first 0x0A01, last 0x1987
     constants:  after "CONSTANT ROM (NUMBER)", "nnn: <16 hex digits>" up to the next "---", each row read
                 back to front; TMC0582-CONST-K.txt + TMC0583-CONST-K.txt = 64 rows of 16 nibbles
     module:     after the first "---", "nnnn: hh hh ..." (decimal address, BCD bytes), into 5000 bytes
   The blob is what host-ref/mkblob.cpp writes and TI59Core::LoadBlob takes: the words big-endian, then the
   1024 nibbles. port/romtexttest.cp checks both against build/ti59rom.bin and build/leisure.bin.
   Header only, so the app (CMain.cp) and the host test share it.
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. GPL-3.0, derived from Calc-U-59. */

#ifndef ROMTEXT_H
#define ROMTEXT_H

#include "TI59Core.h"

enum { kRomTextModuleBytes = 5000 };

struct RomTextLine {                    /* one line, trimmed: p[0..n) */
    const char* p;
    long n;
};

/* the next line from t[*pos..len), trimmed; 0 at the end */
static short RomTextNextLine(const char* t, long len, long* pos, RomTextLine* ln)
{
    long a = *pos, e;
    if (a >= len) return 0;
    for (e = a; e < len && t[e] != '\r' && t[e] != '\n'; e++) ;
    *pos = e;
    if (*pos < len && t[*pos] == '\r') (*pos)++;
    if (*pos < len && t[*pos] == '\n') (*pos)++;
    while (a < e && (t[a] == ' ' || t[a] == '\t')) a++;
    while (e > a && (t[e - 1] == ' ' || t[e - 1] == '\t')) e--;
    ln->p = t + a;
    ln->n = e - a;
    return 1;
}

static short RomTextHexDigit(char c)
{
    if (c >= '0' && c <= '9') return (short) (c - '0');
    if (c >= 'a' && c <= 'f') return (short) (c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (short) (c - 'A' + 10);
    return -1;
}

/* whole token p[0..n) as hex (optional '+', at most 8 digits); -1 if it is not */
static long RomTextHex(const char* p, long n)
{
    long v = 0, i;
    if (n > 0 && p[0] == '+') { p++; n--; }
    if (n <= 0 || n > 8) return -1;
    for (i = 0; i < n; i++) {
        short d = RomTextHexDigit(p[i]);
        if (d < 0) return -1;
        v = v * 16 + d;
    }
    return v;
}

/* whole token as decimal (optional sign); 0 in *ok if it is not */
static long RomTextDec(const char* p, long n, short* ok)
{
    long v = 0, i;
    short neg = 0;
    *ok = 0;
    if (n > 0 && (p[0] == '+' || p[0] == '-')) { neg = (short) (p[0] == '-'); p++; n--; }
    if (n <= 0 || n > 18) return 0;
    for (i = 0; i < n; i++) {
        if (p[i] < '0' || p[i] > '9') return 0;
        v = v * 10 + (p[i] - '0');
    }
    *ok = 1;
    return neg ? -v : v;
}

static short RomTextStartsWith(const RomTextLine* ln, const char* s)
{
    long i;
    for (i = 0; s[i] != 0; i++) if (i >= ln->n || ln->p[i] != s[i]) return 0;
    return 1;
}

static long RomTextColon(const RomTextLine* ln)
{
    long i;
    for (i = 0; i < ln->n; i++) if (ln->p[i] == ':') return i;
    return -1;
}

/* the next token of p[*i..n), split on spaces and tabs; 0 at the end */
static short RomTextToken(const char* p, long n, long* i, const char** tp, long* tn)
{
    while (*i < n && (p[*i] == ' ' || p[*i] == '\t')) (*i)++;
    if (*i >= n) return 0;
    *tp = p + *i;
    while (*i < n && p[*i] != ' ' && p[*i] != '\t') (*i)++;
    *tn = (long) (p + *i - *tp);
    return 1;
}

/* ROM words of one file appended at out[*count..max); 0 on an invalid word or too many */
static short RomTextWords(const char* t, long len, u16* out, long* count, long max)
{
    long pos = 0, c, i;
    short inData = 0;
    RomTextLine ln;
    const char* tp;
    long tn, v;

    while (RomTextNextLine(t, len, &pos, &ln)) {
        if (!inData) { if (RomTextStartsWith(&ln, "---")) inData = 1; continue; }
        if (ln.n == 0) continue;
        c = RomTextColon(&ln);
        if (c < 0) continue;
        { RomTextLine a = ln; a.n = c; while (a.n > 0 && (a.p[a.n - 1] == ' ' || a.p[a.n - 1] == '\t')) a.n--;
          if (a.n != 4 || RomTextHex(a.p, a.n) < 0) continue; }
        i = c + 1;
        while (RomTextToken(ln.p, ln.n, &i, &tp, &tn)) {
            v = RomTextHex(tp, tn);
            if (v < 0 || v > 0xFFFF || *count >= max) return 0;
            out[(*count)++] = (u16) (v & 0x1FFF);
        }
    }
    return 1;
}

/* constant rows of one file appended at out[*rows * 16..), max rows; 0 on an invalid nibble */
static short RomTextConstants(const char* t, long len, u8* out, long* rows, long max)
{
    long pos = 0, c, a, e, k;
    short inNumber = 0, ok;
    RomTextLine ln;

    while (RomTextNextLine(t, len, &pos, &ln)) {
        { long j; short hit = 0;                       /* the header may stand anywhere in its line */
          for (j = 0; j + 21 <= ln.n && !hit; j++) {
              RomTextLine s; s.p = ln.p + j; s.n = ln.n - j;
              if (RomTextStartsWith(&s, "CONSTANT ROM (NUMBER)")) hit = 1;
          }
          if (hit) { inNumber = 1; continue; } }
        if (RomTextStartsWith(&ln, "---")) { if (inNumber) break; continue; }
        if (!inNumber || ln.n == 0) continue;
        c = RomTextColon(&ln);
        if (c < 0) continue;
        e = c;
        while (e > 0 && (ln.p[e - 1] == ' ' || ln.p[e - 1] == '\t')) e--;
        if (e == 0) continue;
        RomTextDec(ln.p, e, &ok);
        if (!ok) continue;
        a = c + 1;
        while (a < ln.n && (ln.p[a] == ' ' || ln.p[a] == '\t')) a++;
        if (ln.n - a != 16) continue;
        if (*rows >= max) return 0;
        for (k = 0; k < 16; k++) {
            short d = RomTextHexDigit(ln.p[ln.n - 1 - k]);
            if (d < 0) return 0;
            out[*rows * 16 + k] = (u8) d;
        }
        (*rows)++;
    }
    return 1;
}

/* a module into out[0..5000), zero-filled; the bytes used, 0 if there is no data */
static long RomTextModule(const char* t, long len, u8* out)
{
    long pos = 0, c, i, used = 0, k, tn, v, idx, addr, n;
    short inData = 0, ok;
    RomTextLine ln;
    const char* tp;

    for (k = 0; k < kRomTextModuleBytes; k++) out[k] = 0;
    while (RomTextNextLine(t, len, &pos, &ln)) {
        if (!inData) { if (RomTextStartsWith(&ln, "---")) inData = 1; continue; }
        if (ln.n >= 5 && (ln.p[0] == 'A' || ln.p[0] == 'a') && (ln.p[1] == 'D' || ln.p[1] == 'd')
            && (ln.p[2] == 'D' || ln.p[2] == 'd') && (ln.p[3] == 'R' || ln.p[3] == 'r') && ln.p[4] == ':') {
            short data = 0;
            for (k = 0; k + 4 <= ln.n; k++)
                if ((ln.p[k] | 0x20) == 'd' && (ln.p[k + 1] | 0x20) == 'a' && (ln.p[k + 2] | 0x20) == 't'
                    && (ln.p[k + 3] | 0x20) == 'a') data = 1;
            if (data) continue;
        }
        if (ln.n == 0) continue;
        c = RomTextColon(&ln);
        if (c < 0) continue;
        n = c;
        while (n > 0 && (ln.p[n - 1] == ' ' || ln.p[n - 1] == '\t')) n--;
        { long s = 0; while (s < n && (ln.p[s] == ' ' || ln.p[s] == '\t')) s++;
          addr = RomTextDec(ln.p + s, n - s, &ok); }
        if (!ok) continue;
        i = c + 1;
        k = 0;
        while (RomTextToken(ln.p, ln.n, &i, &tp, &tn)) {
            v = RomTextHex(tp, tn);
            if (v < 0 || v > 0xFF) continue;
            idx = addr + k++;
            if (idx >= 0 && idx < kRomTextModuleBytes) {
                out[idx] = (u8) v;
                if (idx + 1 > used) used = idx + 1;
            }
        }
    }
    return used;
}

/* the blob from the five files (ROM 1-3, constants 1-2), each text[i] of size len[i];
   0 if a file is missing or does not read as the TI-59's ROM */
static short RomTextBlob(const char* const text[5], const long len[5], u8* blob, u16* words)
{
    long count = 0, rows = 0, i;
    for (i = 0; i < 3; i++)
        if (text[i] == 0 || !RomTextWords(text[i], len[i], words, &count, kTI59RomWords)) return 0;
    if (count != kTI59RomWords || words[0] != 0x0A01 || words[kTI59RomWords - 1] != 0x1987) return 0;
    for (i = 3; i < 5; i++)
        if (text[i] == 0 || !RomTextConstants(text[i], len[i], blob + 2 * kTI59RomWords, &rows, 64)) return 0;
    if (rows != 64) return 0;
    for (i = 0; i < kTI59RomWords; i++) {
        blob[2 * i] = (u8) (words[i] >> 8);
        blob[2 * i + 1] = (u8) (words[i] & 0xFF);
    }
    return 1;
}

#endif
