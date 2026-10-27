/* main.cp - TI59Test: the acceptance run on the Mac (Symantec C++ 7, "C++ Project" model, ANSI++ console).
   One member file, so the project needs no Add Files: the core and the test run come in by #include.
   ti59rom.bin (host-ref/mkblob.cpp) lies next to the project; the lines go to TI59Test.out, which must
   equal host-ref/reference.txt; the time goes to the console. */

#include "TI59Core.cp"

#define main PortRunMain
#include "portrun.cp"
#undef main

#include <time.h>

int main(void)
{
    static char* argv[3] = {"TI59Test", "ti59rom.bin", 0};
    clock_t t0, t1;
    int r;

    fprintf(stderr, "TI59Test: running the 14 sequences ...\n");
    if (!freopen("TI59Test.out", "w", stdout)) { fprintf(stderr, "cannot write TI59Test.out\n"); return 1; }
    t0 = clock();
    r = PortRunMain(2, argv);
    t1 = clock();
    fclose(stdout);
    fprintf(stderr, "TI59Test: done, result %ld, %ld ticks (%ld per second)\n",
            (long)r, (long)(t1 - t0), (long)CLOCKS_PER_SEC);
    return r;
}
