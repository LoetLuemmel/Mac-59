#!/bin/sh
# build.sh - the host reference (original Calc-U-59 core). Run from the repo root.
set -e
mkdir -p build
c++ -std=c++20 -O2 -Iupstream/Core -Iupstream/Shared \
    upstream/Core/TMC0501.cpp upstream/Core/TI59Machine.cpp upstream/Core/RAM.cpp upstream/Core/ROM.cpp \
    upstream/Shared/RomLoader.cpp host-ref/refrun.cpp -o build/refrun
