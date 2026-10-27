// mkmodule.cpp - a solid-state module's ROM text dump (roms/solid-state/TMC05xx.txt) as the 5000-byte image the
// port loads (TI59Core::LoadLibrary), parsed with the ORIGINAL loader. Leisure Library = TMC0547.
//   build/mkmodule roms/solid-state/TMC0547.txt build/leisure.bin
#include "RomLoader.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "usage: mkmodule <TMC05xx.txt> <out.bin>\n"); return 2; }
    std::ifstream f(argv[1]); std::stringstream s; s << f.rdbuf();
    auto img = cu59::parseModuleTxt(s.str());
    if (!img) { std::fprintf(stderr, "no module data\n"); return 1; }
    std::vector<uint8_t> out(5000, 0);
    for (size_t i = 0; i < img->size() && i < 5000; i++) out[i] = (*img)[i];
    FILE* o = std::fopen(argv[2], "wb"); std::fwrite(out.data(), 1, out.size(), o); std::fclose(o);
    std::printf("%s: %zu bytes used, programs %02X\n", argv[2], img->size(), out[0]);
    return 0;
}
