#!/bin/sh
# build.sh - the port on the host, and the ROM blob it loads. Run from the repo root.
# -std=c++98 and the warnings keep the code near what Symantec C++ 7 accepts; the real check is the Mac.
set -e
mkdir -p build
c++ -std=c++20 -O2 -Iupstream/Core -Iupstream/Shared \
    upstream/Shared/RomLoader.cpp host-ref/mkblob.cpp -o build/mkblob
build/mkblob roms build/ti59rom.bin
c++ -std=c++98 -pedantic -Wall -Wextra -Wno-long-long -O2 -x c++ port/TI59Core.cp -x c++ port/portrun.cp -o build/portrun
# the module image, and the app's reader of Calc-U-59's dumps checked against both blobs (port/RomText.h)
c++ -std=c++20 -O2 -Iupstream/Core -Iupstream/Shared \
    upstream/Shared/RomLoader.cpp host-ref/mkmodule.cpp -o build/mkmodule
build/mkmodule roms/solid-state/TMC0547.txt build/leisure.bin
c++ -std=c++98 -pedantic -Wall -Wextra -Wno-long-long -Wno-deprecated-declarations -O2 -Iport \
    -x c++ port/romtexttest.cp -o build/romtexttest
build/romtexttest roms
