/* cardtexttest.cp - host only: CardTextParse on a text card, printed as 246 hex bytes (8 per line after
   the 4 header bytes), for comparison with Calc-U-59's own parser:
     build/cardtexttest <file.u59>   vs   build/cardparse <file.u59> (Calc-U-59 Shared/CardFile) */
#include "CardText.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    static char buf[20000];
    u8 bank[246];
    long n, i;
    FILE* f = fopen(argv[1], "rb");
    if (!f) return 2;
    n = (long) fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (!CardTextParse(buf, n, bank)) { printf("nil\n"); return 1; }
    for (i = 0; i < 246; i++) printf("%02X%s", bank[i], (i % 8 == 3) ? "\n" : " ");
    printf("\n");
    return 0;
}
