// refrun.cpp — the host reference for the port: the ORIGINAL Calc-U-59 core
// (upstream/) runs fixed key sequences on the TI-59 ROM and prints, per
// sequence, the display and a fingerprint of the CPU's path (FNV-1a over the
// PC after every step) plus the step count. The port in port/ must print the
// same lines.
//
//   host-ref/build.sh && build/refrun > host-ref/reference.txt
//
// Setup: TI-59, ROM + constants, no Master Library module, printer
// disconnected (the Mac has none). Each sequence starts from a fresh boot.
// A key tap = press, 0.1 s of emulated instructions, release, 0.1 s, then run
// to the keyboard scan loop - the same tap as Calc-U-59's tests/TestRoms.hpp,
// but stepped one instruction at a time so every PC enters the fingerprint.

#include "TI59Machine.hpp"
#include "KeyMatrix.hpp"
#include "MachineModelInfo.hpp"
#include "RomLoader.hpp"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using cu59::Model;

namespace {

struct Run {
    std::unique_ptr<TI59Machine> m;
    uint32_t hash = 2166136261u;
    uint32_t steps = 0;

    void step() {
        m->step();
        uint16_t pc = m->pc();
        hash = (hash ^ (pc & 0xFF)) * 16777619u;
        hash = (hash ^ (pc >> 8)) * 16777619u;
        ++steps;
    }
    void stepFor(uint32_t n) { for (uint32_t i = 0; i < n; ++i) step(); }
    bool runToScanLoop(uint32_t maxSteps = 2000000) {
        const auto& range = cu59::info(Model::TI59).scanLoop;
        for (uint32_t i = 0; i < maxSteps; ++i) {
            step();
            if (range.contains(m->pc())) return true;
        }
        return false;
    }
    void tap(int code) {
        uint32_t n = static_cast<uint32_t>(cu59::info(Model::TI59).instructionHz * 0.1);
        cu59::pressMatrix(*m, code);
        stepFor(n);
        cu59::releaseMatrix(*m, code);
        stepFor(n);
        runToScanLoop();
    }
};

std::string displayText(TI59Machine& m) {
    DisplaySnapshot d = m.takeDisplay();
    std::string s;
    for (int i = 11; i >= 0; --i) {
        if (d.digitDuty[i] == 0) continue;
        switch (d.ctrl[i]) {
        case 0: case 1: case 8: case 9: s += static_cast<char>('0' + (d.digits[i] & 0xF) % 10); break;
        case 5: s += d.digits[i] == 0 ? '-' : 'o'; break;
        case 6: s += '-'; break;
        default: break;
        }
    }
    return s;
}

std::unique_ptr<TI59Machine> boot(const char* romDir) {
    auto read = cu59::directoryReader({romDir});
    auto m = std::make_unique<TI59Machine>(cu59::toVariant(Model::TI59));
    auto words = cu59::loadRom(Model::TI59, read);
    m->loadROM(words.data(), words.size());
    auto k = cu59::loadConstants(Model::TI59, read);
    m->loadConstants(k.data(), k.size());
    m->setPrinterConnected(false);
    return m;
}

// Matrix codes, row x 10 + column, as on the keyboard (row 1 = A..E).
enum {
    k2nd = 21, kINV = 22, kLnx = 23, kCE = 24, kCLR = 25,
    kLRN = 31, kXT = 32, kSq = 33, kSqrt = 34, kRecip = 35,
    kSST = 41, kSTO = 42, kRCL = 43, kSUM = 44, kPow = 45,
    kEE = 52, kLP = 53, kRP = 54, kDiv = 55,
    kGTO = 61, k7 = 62, k8 = 63, k9 = 64, kMul = 65,
    k4 = 72, k5 = 73, k6 = 74, kSub = 75,
    kRST = 81, k1 = 82, k2 = 83, k3 = 84, kAdd = 85,
    kRS = 91, k0 = 92, kPt = 93, kNeg = 94, kEq = 95
};

struct Seq { const char* name; std::vector<int> keys; };

} // namespace

int main(int argc, char** argv) {
    const char* romDir = argc > 1 ? argv[1] : "roms";
    const std::vector<Seq> seqs = {
        {"boot",            {}},
        {"aos_2+3x4",       {kCLR, k2, kAdd, k3, kMul, k4, kEq}},
        {"pow_2^10",        {kCLR, k2, kPow, k1, k0, kEq}},
        {"sin30+pi",        {kCLR, k3, k0, k2nd, kSq, kAdd, k2nd, k3, kEq}},
        {"sqrt2",           {kCLR, k2, kSqrt}},
        {"paren",           {kCLR, kLP, k2, kAdd, k3, kRP, kMul, k4, kEq}},
        {"recip0_error",    {kCLR, k0, kRecip}},
        {"sto_rcl",         {kCLR, k5, kSTO, k0, k1, kCLR, kRCL, k0, k1}},
        {"program_x2",      {kCLR, kLRN, kMul, k2, kEq, kRS, kLRN, kRST, k7, kRS}},
        {"ee_1.5e3",        {kCLR, k1, kPt, k5, kEE, k3, kEq}},
        {"lnx_2",           {kCLR, k2, kLnx}},
        {"inv_sin_0.5",     {kCLR, kPt, k5, kINV, k2nd, kSq}},
        {"div_7/3",         {kCLR, k7, kDiv, k3, kEq}},
        {"sub_neg",         {kCLR, k5, kSub, k8, kEq}},
    };
    for (const auto& s : seqs) {
        Run r;
        r.m = boot(romDir);
        bool ok = r.runToScanLoop();
        for (int k : s.keys) r.tap(k);
        // read the display over a fresh window: takeDisplay() integrates since its
        // previous call, which would mix e.g. the LRN display into the result
        r.m->takeDisplay();
        r.stepFor(static_cast<uint32_t>(cu59::info(Model::TI59).instructionHz * 0.1));
        std::printf("%-14s display=%-14s steps=%-8u pcfnv=%08x%s\n", s.name, displayText(*r.m).c_str(),
                    r.steps, r.hash, ok ? "" : " BOOT-FAILED");
    }
    return 0;
}
