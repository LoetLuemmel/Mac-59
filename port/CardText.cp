/* CardText.cp - Calc-U-59's text card format ("Calc-U-59 Card 1.0"), read into a 246-byte bank,
   in the C++ subset of TI59Core. Layout (Calc-U-59 Shared/CardFile.hpp):

     Calc-U-59 Card 1.0
     CUECARD:             (optional; skipped here)
     HEADER:              Partition: 1C / DataType: program|data / Bank: 1-4 / Protection: yes|no / Checksum: 02
     DATA:                30 lines "Rnnn: HH HH HH HH HH HH HH HH" = bank bytes 4..243 in order

   The two byte reversals in Calc-U-59's code (file vs. ti58c.mem, and its register-line encoding)
   cancel out, so a data line's bytes are the bank bytes as written; checked against Calc-U-59's own
   parser on its test cards (port/cardtexttest.cp). '#' starts a comment; keywords are case-insensitive.
   (c) 2026 Pit Förster, with Claude Code via AppleBridge. Derived from Calc-U-59, (c) 2026 Martin Erzberger, GPL-3.0. */

#include "CardText.h"
#include <string.h>

static char CTLower(char c) { return (char) (c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c); }

static short CTHex(char c)
{
    if (c >= '0' && c <= '9') return (short) (c - '0');
    c = CTLower(c);
    if (c >= 'a' && c <= 'f') return (short) (c - 'a' + 10);
    return -1;
}

/* s[0..n) equals the lowercase word w, case-insensitively */
static short CTIs(const char* s, long n, const char* w)
{
    long i;
    for (i = 0; i < n && w[i]; i++) if (CTLower(s[i]) != w[i]) return 0;
    return i == n && w[i] == 0;
}

/* a hex number of 1-2 digits, surrounded by blanks; -1 if none */
static short CTHexByte(const char* s, long n)
{
    long i = 0;
    short v = 0, d, digits = 0;
    while (i < n && s[i] == ' ') i++;
    while (i < n && (d = CTHex(s[i])) >= 0 && digits < 3) { v = (short) (v * 16 + d); i++; digits++; }
    while (i < n && s[i] == ' ') i++;
    return (digits >= 1 && digits <= 2 && i == n) ? v : (short) -1;
}

short CardTextIs(const char* buf, long n)
{
    return n >= 15 && memcmp(buf, "Calc-U-59 Card ", 15) == 0;
}

short CardTextParse(const char* buf, long n, u8* bank)
{
    enum { kNone, kCue, kHeader, kData } section = kNone;
    long pos = 0, start, end, colon, k, i;
    short hasHeader = 0, hasData = 0, row = 0, v, b;
    u8 partition = 0, dataType = 0x11, pageByte = 0x10, protection = 0, checksum = 0;
    const char *line, *val;
    long len, vlen;

    if (!CardTextIs(buf, n)) return 0;
    memset(bank, 0, 246);
    while (pos < n && buf[pos] != '\r' && buf[pos] != '\n') pos++;      /* the magic line */
    while (pos < n) {
        while (pos < n && (buf[pos] == '\r' || buf[pos] == '\n')) pos++;
        start = pos;
        while (pos < n && buf[pos] != '\r' && buf[pos] != '\n') pos++;
        end = pos;
        for (k = start; k < end; k++) if (buf[k] == '#') { end = k; break; }   /* comment */
        while (start < end && (buf[start] == ' ' || buf[start] == '\t')) start++;
        while (end > start && (buf[end - 1] == ' ' || buf[end - 1] == '\t')) end--;
        line = buf + start;
        len = end - start;
        if (len == 0) continue;
        if (CTIs(line, len, "cuecard:")) { section = kCue; continue; }
        if (CTIs(line, len, "header:"))  { section = kHeader; continue; }
        if (CTIs(line, len, "data:"))    { section = kData; hasData = 1; row = 0; continue; }

        colon = -1;
        for (k = 0; k < len; k++) if (line[k] == ':') { colon = k; break; }
        if (section == kHeader) {
            if (colon < 0) continue;
            val = line + colon + 1;
            vlen = len - colon - 1;
            while (vlen > 0 && *val == ' ') { val++; vlen--; }
            k = colon;
            while (k > 0 && line[k - 1] == ' ') k--;
            if (CTIs(line, k, "partition")) {
                v = CTHexByte(val, vlen);
                if (v < 0) return 0;
                partition = (u8) v;
                hasHeader = 1;
            }
            else if (CTIs(line, k, "datatype")) dataType = (u8) (CTIs(val, vlen, "program") ? 0x11 : 0x10);
            else if (CTIs(line, k, "bank")) {
                if (vlen != 1 || val[0] < '1' || val[0] > '4') return 0;
                pageByte = (u8) (0x10 | ((val[0] - '1') * 3));
            }
            else if (CTIs(line, k, "protection")) protection = (u8) (CTIs(val, vlen, "yes") ? 0x10 : 0x00);
            else if (CTIs(line, k, "checksum")) {
                v = CTHexByte(val, vlen);
                if (v < 0) return 0;
                checksum = (u8) v;
            }
        }
        else if (section == kData) {
            if (line[0] != 'R' || colon < 0) return 0;
            k = colon + 1;
            for (i = 0; i < 8; i++) {                       /* exactly 8 hex bytes */
                while (k < len && line[k] == ' ') k++;
                if (k + 1 >= len + 1 || CTHex(line[k]) < 0) return 0;
                b = CTHex(line[k]);
                k++;
                if (k < len && CTHex(line[k]) >= 0) { b = (short) (b * 16 + CTHex(line[k])); k++; }
                if (k < len && line[k] != ' ') return 0;
                if (row < 30) bank[4 + row * 8 + i] = (u8) b;
            }
            while (k < len && line[k] == ' ') k++;
            if (k != len) return 0;
            row++;
        }
    }
    if (!hasHeader || !hasData) return 0;
    bank[0] = partition;
    bank[1] = dataType;
    bank[2] = pageByte;
    bank[3] = protection;
    bank[244] = checksum;
    bank[245] = checksum;
    return 1;
}
