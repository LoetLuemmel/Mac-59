// mkblob.cpp - turns the ROM text dumps into the one binary the port loads: the 6144 13-bit words of the
// TI-59 ROM, big-endian, followed by the 64 x 16 constant nibbles (13312 bytes). Parsed with the ORIGINAL
// loader (upstream/Shared/RomLoader), so the port and the reference start from the same bytes. On the Mac
// the same file becomes a resource.
//
//   build/mkblob roms build/ti59rom.bin

#include "MachineModelInfo.hpp"
#include "RomLoader.hpp"

#include <cstdio>

using cu59::Model;

int main(int argc, char** argv) {
    const char* romDir = argc > 1 ? argv[1] : "roms";
    const char* out = argc > 2 ? argv[2] : "build/ti59rom.bin";
    auto read = cu59::directoryReader({romDir});
    auto words = cu59::loadRom(Model::TI59, read);
    auto k = cu59::loadConstants(Model::TI59, read);
    if (words.size() != 6144 || k.size() != 1024) {
        std::fprintf(stderr, "unexpected sizes: %zu words, %zu constant bytes\n", words.size(), k.size());
        return 1;
    }
    FILE* f = std::fopen(out, "wb");
    if (!f) { std::perror(out); return 1; }
    for (uint16_t w : words) { std::fputc(w >> 8, f); std::fputc(w & 0xFF, f); }
    std::fwrite(k.data(), 1, k.size(), f);
    std::fclose(f);
    std::printf("%s: %zu bytes\n", out, words.size() * 2 + k.size());
    return 0;
}
