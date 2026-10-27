/* romtexttest.cp - host only: port/RomText.h against the blobs the original loader made
   (build/ti59rom.bin by host-ref/mkblob, build/leisure.bin by host-ref/mkmodule), for the dumps with
   LF, CR and CR LF line ends; and that a damaged dump is refused.
     build/romtexttest roms        exit 0 = every case as expected
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. */

#include "RomText.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* ReadAll(const char* path, long* len)
{
    FILE* f = fopen(path, "rb");
    char* b;
    if (!f) return 0;
    fseek(f, 0, SEEK_END); *len = ftell(f); fseek(f, 0, SEEK_SET);
    b = (char*) malloc((size_t) *len + 1);
    fread(b, 1, (size_t) *len, f); fclose(f);
    return b;
}

/* the text with every LF replaced by mode: 0 LF (as read), 1 CR, 2 CR LF */
static char* Ends(const char* t, long n, short mode, long* out)
{
    char* b = (char*) malloc((size_t) n * 2 + 1);
    long i, k = 0;
    for (i = 0; i < n; i++) {
        if (t[i] == '\n' && mode == 1) b[k++] = '\r';
        else if (t[i] == '\n' && mode == 2) { b[k++] = '\r'; b[k++] = '\n'; }
        else b[k++] = t[i];
    }
    *out = k;
    return b;
}

static short fails;

static void Check(const char* name, short ok)
{
    printf("%-4s %s\n", ok ? "ok" : "FAIL", name);
    if (!ok) fails++;
}

int main(int argc, char** argv)
{
    static const char* files[5] = { "TMC0582.txt", "TMC0583.txt", "TMC0571B.txt",
                                    "TMC0582-CONST-K.txt", "TMC0583-CONST-K.txt" };
    const char* dir = argc > 1 ? argv[1] : "roms";
    char path[512];
    char* raw[5];
    long rawLen[5], refLen, modRefLen, i, used;
    char* ref = ReadAll("build/ti59rom.bin", &refLen);
    char* modRef = ReadAll("build/leisure.bin", &modRefLen);
    static u8 blob[kTI59BlobBytes];
    static u16 words[kTI59RomWords];
    static u8 module[kRomTextModuleBytes];
    short mode;

    if (!ref || !modRef) { fprintf(stderr, "build/ti59rom.bin or build/leisure.bin missing\n"); return 2; }
    for (i = 0; i < 5; i++) {
        sprintf(path, "%s/%s", dir, files[i]);
        raw[i] = ReadAll(path, &rawLen[i]);
        if (!raw[i]) { fprintf(stderr, "%s missing\n", path); return 2; }
    }
    for (mode = 0; mode < 3; mode++) {
        const char* text[5];
        long len[5];
        char name[80];
        for (i = 0; i < 5; i++) text[i] = Ends(raw[i], rawLen[i], mode, &len[i]);
        memset(blob, 0, sizeof blob);
        sprintf(name, "ROM blob = build/ti59rom.bin, line ends %s", mode == 0 ? "LF" : mode == 1 ? "CR" : "CR LF");
        Check(name, (short) (RomTextBlob(text, len, blob, words) && refLen == kTI59BlobBytes
                             && memcmp(blob, ref, kTI59BlobBytes) == 0));
    }
    {
        long mlen, ml;
        char* m;
        sprintf(path, "%s/solid-state/TMC0547.txt", dir);
        m = ReadAll(path, &mlen);
        if (!m) { fprintf(stderr, "%s missing\n", path); return 2; }
        for (mode = 0; mode < 3; mode++) {
            char* t = Ends(m, mlen, mode, &ml);
            char name[80];
            used = RomTextModule(t, ml, module);
            sprintf(name, "module = build/leisure.bin, line ends %s (%ld bytes used)",
                    mode == 0 ? "LF" : mode == 1 ? "CR" : "CR LF", used);
            Check(name, (short) (used > 0 && modRefLen == kRomTextModuleBytes
                                 && memcmp(module, modRef, kRomTextModuleBytes) == 0));
        }
    }
    {   /* refused: a missing file, the files in the wrong order, a damaged word */
        const char* text[5];
        long len[5];
        for (i = 0; i < 5; i++) { text[i] = raw[i]; len[i] = rawLen[i]; }
        text[2] = 0;
        Check("refused: TMC0571B.txt missing", (short) !RomTextBlob(text, len, blob, words));
        text[2] = raw[2];
        text[0] = raw[1]; len[0] = rawLen[1]; text[1] = raw[0]; len[1] = rawLen[0];
        Check("refused: TMC0582 and TMC0583 swapped", (short) !RomTextBlob(text, len, blob, words));
        text[0] = raw[0]; len[0] = rawLen[0]; text[1] = raw[1]; len[1] = rawLen[1];
        {
            char* bad = (char*) malloc((size_t) rawLen[0] + 1);
            char* p;
            memcpy(bad, raw[0], (size_t) rawLen[0]);
            bad[rawLen[0]] = 0;
            p = strstr(bad, "0A0A");
            if (p) p[1] = 'X';
            text[0] = bad;
            Check("refused: a damaged word", (short) (p && !RomTextBlob(text, len, blob, words)));
        }
        text[0] = raw[0];
        text[3] = raw[4]; len[3] = rawLen[4]; text[4] = 0;
        Check("refused: a constants file missing", (short) !RomTextBlob(text, len, blob, words));
    }
    printf("%s\n", fails ? "NICHT BESTANDEN" : "BESTANDEN");
    return fails ? 1 : 0;
}
