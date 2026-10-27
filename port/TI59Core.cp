/* TI59Core.cp - the TMC0501 CPU and the TI-59 machine around it; see TI59Core.h.

   The code follows upstream/Core/TMC0501.cpp function by function and keeps its order of effects, because
   the acceptance test compares the program counter after every single instruction. Comments carry the
   hardware explanation in short; the long form is upstream's. Port (c) 2026 Pit Förster, with Claude Code via AppleBridge.
   Derived from Calc-U-59, (c) 2026 Martin
   Erzberger, GPL-3.0. */

#include "TI59Core.h"
#include <string.h>

/* Internal CPU flags. COND must stay at bit 11: the branch test XORs flags with the opcode. */
#define FLG_IDLE      0x0001u   /* display / idle mode: steps weigh 4 */
#define FLG_HOLD      0x0002u   /* re-execute the same instruction (WAIT, KEY scan-all) */
#define FLG_JUMP      0x0004u   /* a branch ran: the next non-branch restores COND */
#define FLG_RECALL    0x0010u   /* next MOV dst,#0 delivers SCOM[REG_ADDR] */
#define FLG_STORE     0x0020u   /* after the next ALU op, Sout -> SCOM[REG_ADDR] */
#define FLG_RAM_OP    0x0040u   /* next Sout encodes a RAM operation */
#define FLG_RAM_READ  0x0080u   /* next MOV dst,#0 delivers RAM[RAM_ADDR] */
#define FLG_RAM_WRITE 0x0100u   /* after the next ALU op, Sout -> RAM[RAM_ADDR] */
#define FLG_EXT_VALID 0x0200u   /* EXT valid for one more cycle */
#define FLG_IO_VALID  0x0400u   /* Sout valid for one more cycle */
#define FLG_COND      0x0800u   /* condition true */
#define FLG_BUSY      0x8000u   /* printer busy */

/* PC-100 timing in instruction cycles (upstream TMC0501.hpp) */
#define kPrinterLineCycles     4165UL
#define kPrinterFeedCycles     1370UL
#define kPrinterRowCycles      508UL
#define kPrinterRowStartCycles 18UL
#define kPrinterColumnCycles   64UL

const u32 TI59Core::kPolledHardware = 0x40000000UL;

/* start, end, cpos, cval per field-mask index; start 0xFF = invalid */
const TI59Core::MaskInfo TI59Core::maskInfo[16] = {
    {0xFF, 0,  0,    0},   /* 0: invalid */
    {  0, 15,  0,    0},   /* 1: ALL */
    {  0,  0,  0,    0},   /* 2: DPT */
    {  0,  0,  0,    1},   /* 3: DPT #1 */
    {  0,  0,  0, 0x0C},   /* 4: DPT #C */
    {  3,  3,  3,    1},   /* 5: LLSD #1 */
    {  1,  2,  1,    0},   /* 6: EXP */
    {  1,  2,  1,    1},   /* 7: EXP #1 */
    {0xFF, 0,  0,    0},   /* 8: invalid */
    {  3, 15,  3,    0},   /* 9: MANT */
    {0xFF, 0,  0,    0},   /* A: invalid */
    {  3, 15,  3,    5},   /* B: MLSD #5 */
    {  1, 15,  1,    0},   /* C: MAEX */
    {  1, 15,  3,    1},   /* D: MAEX, #1 at digit 3 */
    {  1, 15, 15,    1},   /* E: MMSD #1 */
    {  1, 15,  1,    1},   /* F: MAEX #1 */
};

/* Printer character codes that are one ASCII character (others 0): enough to map the function
   mnemonics below back to codes, which is all upstream's prnCharToCode() does. */
static const char kPrnAscii[65] =
    " 0123456789ABCDE-FGHIJKLMNOPQRST.UVWXYZ+x*\0\0e()," "\0%\0/='\0\0" "\0?\0!\0\0\0\0";

static u8 PrnCharToCode(char c)
{
    short i;
    for (i = 0; i < 64; i++)
        if (kPrnAscii[i] == c) return (u8)i;
    return 0;
}

static const struct { u8 code; char str[4]; } kPrnStr[] = {
    {0x00, "   "}, {0x11, " = "}, {0x12, " - "}, {0x13, " + "}, {0x16, " / "}, {0x17, " x "},
    {0x1A, "xsY"}, {0x1B, "Y^x"}, {0x21, "CLR"}, {0x22, "INV"}, {0x23, "DPT"}, {0x26, "CE "},
    {0x27, "+/-"}, {0x2D, "EE "}, {0x31, "e^x"}, {0x33, "x^2"}, {0x36, "1/x"}, {0x3C, "sX "},
    {0x3D, "X_Y"}, {0x51, "LNX"}, {0x53, "PRM"}, {0x54, " % "}, {0x56, "COS"}, {0x57, "SIN"},
    {0x5D, "TAN"}, {0x61, "SUM"}, {0x66, "STO"}, {0x67, "pi "}, {0x68, "RCL"}, {0x69, "S+ "},
    {0x70, "ERR"}, {0x71, " { "}, {0x72, " ) "}, {0x73, "LRN"}, {0x74, "RUN"}, {0x76, "HLT"},
    {0x78, "STP"}, {0x7A, "GTO"}, {0x7C, "IF "}, {0x00, ""}
};

/* -- Construction, power-on, reset ------------------------------------------- */

TI59Core::TI59Core()
{
    memset(rom, 0, sizeof(rom));
    memset(constant, 0, sizeof(constant));
    memset(libData, 0, sizeof(libData));
    printerConnected = 1;
    printerTrace = 0;
    PowerOn();
}

void TI59Core::LoadBlob(const u8* blob)
{
    short i;
    for (i = 0; i < kTI59RomWords; i++)
        rom[i] = (u16)((((u16)blob[2 * i] << 8) | blob[2 * i + 1]) & 0x1FFF);
    memcpy(constant, blob + 2 * kTI59RomWords, kTI59ConstBytes);
}

void TI59Core::LoadLibrary(const u8* data, u32 count)
{
    if (count > 5000) count = 5000;
    memset(libData, 0, sizeof(libData));
    memcpy(libData, data, (size_t)count);
}

/* Every member upstream gives a default initialiser, at that value. */
void TI59Core::ClearState()
{
    memset(ram, 0, sizeof(ram));
    memset(A, 0, 16); memset(B, 0, 16); memset(C, 0, 16); memset(D, 0, 16); memset(E, 0, 16);
    memset(SCOM, 0, sizeof(SCOM));
    memset(Sout, 0, sizeof(Sout));
    memset(key, 0, sizeof(key));
    KR = SR = fA = fB = EXT = PREG = 0;
    flags = 0;
    R5 = digit = 0;
    addr = 0;
    REG_ADDR = RAM_ADDR = RAM_OP = 0;
    RAM_MASK.start = 0xFF; RAM_MASK.end = 0; RAM_MASK.cpos = 0; RAM_MASK.cval = 0;
    lastStepWasHWPoll = 0;
    libAddr = 0; libAddrReadPos = 0; libAddrWasWriting = 0;
    cardSwitchCol = 10;
    cardPresent = waitingForCard = 0;
    memset(cardFullData, 0, sizeof(cardFullData));
    memset(cardBankBuffer, 0, sizeof(cardBankBuffer));
    cardPtr = 0; cardMode = 0; lastWrittenBank = -1;
    memset(prnCodeBuf, 0, sizeof(prnCodeBuf));
    prnPtr = 0; prnReady = 0; prnBusyCycles = 0;
    memset(&prnPending, 0, sizeof(prnPending)); prnPending.rowCount = 7;
    prnHasPending = 0;
    prnHead = prnCount = 0;
    memset(segA, 0, sizeof(segA)); memset(segB, 0, sizeof(segB));
    memset(litTicks, 0, sizeof(litTicks)); memset(dpTicks, 0, sizeof(dpTicks));
    memset(&lastDisplay, 0, sizeof(lastDisplay));
    suppressedMask = 0;
    zeroSuppressRunning = 1;
    cSteps = pollSteps = 0;
}

/* TI59Machine's constructor: fresh CPU (no reset()), card switch pressed = no card, printer sense line. */
void TI59Core::PowerOn()
{
    ClearState();
    PressKey(4, cardSwitchCol);
    if (printerConnected) key[0] |= (1u << 2); else key[0] &= ~(1u << 2);
}

void TI59Core::Reset()
{
    memset(A, 0, 16); memset(B, 0, 16); memset(C, 0, 16); memset(D, 0, 16); memset(E, 0, 16);
    memset(SCOM, 0, sizeof(SCOM));
    memset(Sout, 0, sizeof(Sout));
    memset(key, 0, sizeof(key));
    KR = SR = fA = fB = EXT = PREG = libAddr = 0;
    libAddrReadPos = 0;
    libAddrWasWriting = 0;
    R5 = digit = RAM_ADDR = RAM_OP = REG_ADDR = 0;
    addr = 0;
    flags = FLG_COND;
    memset(segA, 0, sizeof(segA)); memset(segB, 0, sizeof(segB));
    memset(litTicks, 0, sizeof(litTicks)); memset(dpTicks, 0, sizeof(dpTicks));
    memset(&lastDisplay, 0, sizeof(lastDisplay));
    suppressedMask = 0;
    zeroSuppressRunning = 1;
    cardPresent = waitingForCard = 0;
    memset(cardFullData, 0, sizeof(cardFullData));
    memset(cardBankBuffer, 0, sizeof(cardBankBuffer));
    cardPtr = 0; cardMode = 0; lastWrittenBank = -1;
    prnPtr = 0;
    prnReady = 0;
    FlushPendingIfActive();
    prnBusyCycles = 0;
    /* TI59Machine::reset() */
    PressKey(4, cardSwitchCol);
    if (printerConnected) key[0] |= (1u << 2); else key[0] &= ~(1u << 2);
    if (printerTrace) key[15] |= (1u << 2); else key[15] &= ~(1u << 2);
}

/* -- Masked register helpers ------------------------------------------------- */

void TI59Core::ReadRegMasked(u8* dst, short reg, const MaskInfo& m)
{
    short i;
    memset(dst, 0, 16);
    if (reg >= 0 && reg < kRamRegs && m.start != 0xFF)
        for (i = m.start; i <= m.end && i < 16; i++) dst[i] = ram[reg][i];
}

void TI59Core::WriteRegMasked(short reg, const u8* src, const MaskInfo& m)
{
    short i;
    if (m.start != 0xFF && reg < kRamRegs)
        for (i = m.start; i <= m.end && i < 16; i++) ram[reg][i] = src[i];
}

void TI59Core::ReadScomMasked(u8* dst, short reg, const MaskInfo& m)
{
    short i;
    memset(dst, 0, 16);
    if (reg < 16 && m.start != 0xFF)
        for (i = m.start; i <= m.end && i < 16; i++) dst[i] = SCOM[reg][i];
}

void TI59Core::ClearRamRegs(short reg, short count)
{
    if (reg < 0 || reg >= kRamRegs) return;
    if (reg + count > kRamRegs) count = (short)(kRamRegs - reg);
    memset(ram[reg], 0, (size_t)count * 16);
}

/* -- Keys -------------------------------------------------------------------- */

void TI59Core::PressKey(short row, short col)
{
    if (col >= 0 && col < 16 && row >= 0 && row < 7) key[col] |= (u8)(1u << row);
}

void TI59Core::ReleaseKey(short row, short col)
{
    if (col >= 0 && col < 16 && row >= 0 && row < 7) key[col] &= (u8)~(1u << row);
}

/* matrix code row*10 + col (row 1-9 top to bottom, col 1-5); the column is a K-line, the row a digit slot */
static const short kKBits[6] = {0, 1, 2, 3, 5, 6};

static short ValidMatrix(short code)
{
    return code / 10 >= 1 && code / 10 <= 9 && code % 10 >= 1 && code % 10 <= 5;
}

void TI59Core::PressMatrix(short code)
{
    if (ValidMatrix(code)) PressKey(kKBits[code % 10], (short)(code / 10));
}

void TI59Core::ReleaseMatrix(short code)
{
    if (ValidMatrix(code)) ReleaseKey(kKBits[code % 10], (short)(code / 10));
}

/* -- Display ----------------------------------------------------------------- */

void TI59Core::TakeDisplay(TI59Display* out)
{
    TI59Display r;
    u16 driven = 0;
    short i;
    u32 cS = cSteps, pS = pollSteps;
    cSteps = 0;
    pollSteps = 0;
    if (pS == 0) { *out = lastDisplay; return; }
    for (i = 0; i < 12; i++) {
        r.digits[i] = (u8)(segA[i] & 0x0F);
        r.ctrl[i] = (u8)(segB[i] & 0x0F);
        r.litTicks[i] = litTicks[i];
        r.dpTicks[i] = dpTicks[i];
        if (litTicks[i]) driven |= (u16)(1u << i);
        litTicks[i] = 0;
        dpTicks[i] = 0;
    }
    r.suppressedMask = (u16)(suppressedMask & ~driven);
    r.pollTicks = pS;
    r.cTicks = cS;
    lastDisplay = r;
    *out = r;
}

/* -- BCD digit-serial ALU ---------------------------------------------------- */
/* Digit 0 wraps at 16, digits 1-15 at 10. Sout gets every digit of the field, 0 outside it. Carry out
   of the field clears COND for add/sub (not for shifts). R5 takes the result digit at cpos. */

void TI59Core::Alu(u8* dst, const u8* srcX, const u8* srcY, const MaskInfo& m, short op)
{
    u8 carry = 0, shl = 0, sum, shr;
    short i;
    for (i = 0; i <= 15; i++) {
        if (i == m.start) { shl = carry = 0; }
        sum = 0;
        if (srcY) sum = srcY[i];
        if (i == m.cpos) sum |= m.cval;
        shr = sum;
        sum = (u8)(sum + carry);
        if (op >= ALU_SUB) sum = (u8)(-sum);
        if (srcX) { sum = (u8)(sum + srcX[i]); shr |= srcX[i]; }

        Sout[i] = (i >= m.start && i <= m.end) ? (u8)(sum & 0x0F) : (u8)0;

        if (i == 0) {
            carry = (u8)(sum >= 0x10 ? 1 : 0);
            if (carry) sum &= 0x0F;
        } else {
            carry = (u8)(sum >= 10 ? 1 : 0);
            if (carry) {
                if (op < ALU_SUB) sum = (u8)(sum - 10);
                else              sum = (u8)(sum + 10);
            }
        }

        if (i >= m.start && i <= m.end) {
            if (i == m.cpos) R5 = sum;
            if (dst) {
                if (op == ALU_SHL) dst[i] = shl;
                else if (op == ALU_SHR) {
                    if (i > m.start) dst[i - 1] = shr;
                    if (i == m.end)  dst[i] = 0;
                } else dst[i] = sum;
                shl = sum;
            }
            if (i == m.end && !(op & 0x1) && carry) flags &= ~FLG_COND;
        }
    }
}

void TI59Core::Xch(u8* a, u8* b, const MaskInfo& m)
{
    short i;
    u8 t;
    if (m.start == 0xFF) return;
    for (i = m.start; i <= m.end; i++) { t = a[i]; a[i] = b[i]; b[i] = t; }
}

/* -- Main instruction dispatch ----------------------------------------------- */

u32 TI59Core::Step()
{
    u16 opcode, offs, mask, bit;
    u8 wasHWPoll, taken, backward, kmask, kb, m2;
    u32 w;

    /* printer busy countdown */
    if (prnBusyCycles > 0) {
        if (--prnBusyCycles == 0) {
            flags &= ~FLG_BUSY;
            FlushPendingCodeLine(kPrinterLineCycles);
        }
    }

    /* fetch; the first non-branch after a branch chain restores COND */
    opcode = (addr < kTI59RomWords) ? rom[addr] : (u16)0;
    if (!(opcode & 0x1000) && (flags & FLG_JUMP)) {
        flags &= ~FLG_JUMP;
        flags |= FLG_COND;
    }

    wasHWPoll = lastStepWasHWPoll;
    lastStepWasHWPoll = 0;

    digit = (u8)(digit ? digit - 1 : 15);
    flags &= ~FLG_HOLD;

    if (flags & FLG_EXT_VALID) flags &= ~FLG_EXT_VALID;
    else                       EXT = 0;
    if (flags & FLG_IO_VALID) flags &= ~FLG_IO_VALID;
    else                      memset(Sout, 0, sizeof(Sout));

    if (opcode & 0x1000) {
        /* branch: taken when COND matches opcode bit 11; offset bits 10:1, bit 0 = backward */
        flags |= FLG_JUMP;
        taken = (u8)(!((flags ^ opcode) & FLG_COND));
        backward = (u8)((opcode & 0x0001) != 0);
        if (taken) {
            offs = (u16)((opcode >> 1) & 0x3FFu);
            if (backward) addr = (u16)(addr - offs);
            else          addr = (u16)(addr + offs);
        } else {
            addr++;
        }
        PostOperation();
        w = StepWeight();
        if ((flags & FLG_IDLE) ? (fA & 0x4000u) : fA) cSteps++;
        pollSteps += w;
        if (wasHWPoll && taken && backward) w |= kPolledHardware;
        return w;
    }

    switch (opcode & 0x0F00) {

    case 0x0000:    /* flag operations on fA / fB / KR; bit index in opcode bits 7:4 */
        bit = (u16)((opcode >> 4) & 0x000Fu);
        mask = (u16)(1u << bit);
        switch (opcode & 0x000Fu) {
        case 0x0: if (fA & mask) flags &= ~FLG_COND; break;
        case 0x1: fA |= mask; break;
        case 0x2: fA &= (u16)~mask; break;
        case 0x3: fA ^= mask; break;
        case 0x4: if ((fA ^ fB) & mask) { fA ^= mask; fB ^= mask; } break;
        case 0x5: KR |= mask; break;
        case 0x6: if ((fA ^ fB) & mask) fA ^= mask; break;
        case 0x7: fA = (u16)((fA & ~0x001Eu) | ((R5 & 0x0Fu) << 1)); break;
        case 0x8: if (fB & mask) flags &= ~FLG_COND; break;
        case 0x9: fB |= mask; break;
        case 0xA: fB &= (u16)~mask; break;
        case 0xB: fB ^= mask; break;
        case 0xC: if (!((fA ^ fB) & mask)) flags &= ~FLG_COND; break;
        case 0xD: KR &= (u16)~mask; break;
        case 0xE: if ((fA ^ fB) & mask) fB ^= mask; break;
        case 0xF: fB = (u16)((fB & ~0x001Eu) | ((R5 & 0x0Fu) << 1)); break;
        default: break;
        }
        break;

    case 0x0800:    /* keyboard scan: 7 K-lines, active low in bits 2:0 and 6:3 (shifted by one) */
        kmask = (u8)((((opcode & 0x07u) | ((opcode >> 1) & 0x78u)) ^ 0x7Fu) & key[digit]);
        if (kmask & (u8)(kmask - 1)) kmask = 0;   /* two keys at once: none */
        if (!(opcode & 0x0008u)) {
            /* scan-all: hold until a key or digit 0 */
            if (key[digit] & kmask) {
                kb = 0; m2 = kmask;
                while (!(m2 & 1)) { kb++; m2 >>= 1; }
                flags &= ~FLG_COND;
                KR = (u16)(((u16)digit << 4) | (((u16)kb << 8) & 0x0700u));
            } else if (digit) {
                flags |= FLG_HOLD;
            }
        } else {
            /* test-row */
            if (key[digit] & kmask) flags &= ~FLG_COND;
        }
        lastStepWasHWPoll = 1;
        break;

    case 0x0A00:
        ExecControl(opcode);
        break;

    default:
        ExecALU(opcode);
        break;
    }

    PostOperation();

    /* HOLD wins over the PREG redirect */
    if (!(flags & FLG_HOLD)) {
        if (PREG) { addr = PREG; PREG = 0; }
        else      addr++;
    }
    w = StepWeight();
    if ((flags & FLG_IDLE) ? (fA & 0x4000u) : fA) cSteps++;
    pollSteps += w;

    /* SET KR[1] latches a program address: the redirect fires after the next instruction */
    if (KR & 0x2) {
        PREG = (u16)((KR >> 4) | ((KR & 0x1) << 12));
        KR &= (u16)~0x2u;
    }
    return w;
}

/* hi nibble 0xA: WAIT, IDLE, KR, R5, peripherals, library, STO/RCL */
void TI59Core::ExecControl(u16 opcode)
{
    u8 bits74;
    u16 flagReg, t;
    static const u16 kDivisors[4] = {1, 10, 100, 1000};

    switch (opcode & 0x000Fu) {

    case 0x0:   /* WAIT Dn */
        if (digit != ((opcode >> 4) & 0x000Fu)) flags |= FLG_HOLD;
        break;

    case 0x1: flags &= ~FLG_IDLE; break;   /* CLR IDL */
    case 0x2: fA = 0; break;               /* CLR fA */
    case 0x3: break;                       /* WAIT BUSY: no-op */

    case 0x4:   /* INC KR */
        KR = (u16)(KR + 0x0010u);
        if (!(KR & 0xFFF0u)) KR ^= 0x0001u;
        break;

    case 0x5:   /* TST KR[a] */
        if (KR & (1u << ((opcode >> 4) & 0x000Fu))) flags &= ~FLG_COND;
        break;

    case 0x6:   /* MEMWR / MEMRD (TI-58C, never in the TI-59 ROM) or MOV R5,fA|fB[1..4] */
        bits74 = (u8)((opcode >> 4) & 0x000Fu);
        if (bits74 == 0x7) {
            RAM_ADDR = (u8)((Sout[1] * 16u) + Sout[0]);
            if (RAM_ADDR < kRamRegs) flags |= FLG_RAM_WRITE;
        } else if (bits74 == 0x8) {
            RAM_ADDR = (u8)((Sout[1] * 16u) + Sout[0]);
            if (RAM_ADDR < kRamRegs) flags |= FLG_RAM_READ;
        } else {
            flagReg = (opcode & 0x0010u) ? fB : fA;
            R5 = (u8)((flagReg >> 1) & 0x0Fu);
        }
        break;

    case 0x7: R5 = (u8)((opcode >> 4) & 0x000Fu); break;   /* MOV R5,#n */

    case 0x8: ExecPeripheral(opcode); break;

    case 0x9: flags |= FLG_IDLE; break;    /* SET IDL */
    case 0xA: fB = 0; break;               /* CLR fB */

    case 0xB:   /* TST BUSY: card switch (bit 4 of its slot) or printer busy clears COND */
        if (!cardPresent && digit == cardSwitchCol && (key[digit] & (1u << 4)))
            waitingForCard = 1;
        if ((key[digit] & (1u << 4)) || (flags & FLG_BUSY))
            flags &= ~FLG_COND;
        if ((key[digit] & (1u << 4)) && !(flags & FLG_BUSY)) lastStepWasHWPoll = 1;
        break;

    case 0xC: KR = (u16)((KR & 0x000Fu) | EXT); break;      /* MOV KR,EXT[4..15] */
    case 0xD: t = KR; KR = SR; SR = t; break;              /* XCH KR,SR */

    case 0xE:   /* library module */
        switch (opcode & 0x00F0u) {
        case 0x00:  /* IN LIB */
            EXT = (u16)((u16)libData[libAddr++] << 4);
            flags |= FLG_EXT_VALID;
            libAddr %= 5000;
            break;
        case 0x10:  /* OUT LIB_PC: shift in one decimal digit at the top */
            if (!libAddrWasWriting) libAddrReadPos = 0;
            libAddr = (u16)((libAddr / 10) + (((KR >> 4) & 0xFu) * 1000));
            libAddrReadPos = (u8)((libAddrReadPos + 1) % 4);
            libAddrWasWriting = 1;
            break;
        case 0x20:  /* IN LIB_PC */
            if (libAddrWasWriting) libAddrReadPos = 0;
            EXT = (u16)(((libAddr / kDivisors[libAddrReadPos]) % 10) << 4);
            flags |= FLG_EXT_VALID;
            libAddrReadPos = (u8)((libAddrReadPos + 1) % 4);
            libAddrWasWriting = 0;
            break;
        case 0x30:  /* IN LIB_HIGH */
            EXT = (u16)(libData[libAddr] & 0xF0u);
            flags |= FLG_EXT_VALID;
            break;
        default: break;
        }
        break;

    case 0xF:   /* STO / RCL: register from Sout[0], data on the next cycle */
        switch (opcode & 0x00F0u) {
        case 0x00: REG_ADDR = (u8)(Sout[0] & 0x0Fu); flags |= FLG_STORE; break;
        case 0x10: flags |= FLG_RECALL; REG_ADDR = (u8)(Sout[0] & 0x0Fu); break;
        default: break;
        }
        break;

    default: break;
    }
}

/* 0x0A8x: card reader, printer, RAM operation */
void TI59Core::ExecPeripheral(u16 opcode)
{
    u8 code, slot;
    short i, k, bank;

    switch (opcode & 0x00F0u) {
    case 0x00: R5 = (u8)((KR >> 4) & 0x000Fu); break;                       /* MOV R5,KR[4..7] */
    case 0x10: KR = (u16)((KR & ~0x00F0u) | ((u16)R5 << 4)); break;        /* MOV KR[4..7],R5 */
    case 0x20:  /* IN CRD */
        if (cardPresent && cardMode == 1 && cardPtr < 246)
            EXT = (u16)((u16)cardBankBuffer[cardPtr++] << 4);
        else
            EXT = 0;
        flags |= FLG_EXT_VALID;
        break;
    case 0x30:  /* OUT CRD */
        if (cardPresent && cardMode == 2 && cardPtr < 246)
            cardBankBuffer[cardPtr++] = (u8)((KR >> 4) & 0xFFu);
        break;
    case 0x40:  /* CRD_OFF: the card leaves the reader */
        if (cardMode == 2) {
            bank = (short)((cardBankBuffer[2] & 0x0Fu) / 3);
            if (bank >= 0 && bank < 4) {
                memcpy(cardFullData + bank * 246, cardBankBuffer, 246);
                lastWrittenBank = bank;
            }
        } else if (cardMode == 1) {
            cardBankBuffer[2] = (u8)((cardBankBuffer[2] + 3) & 0x0F);
        }
        cardMode = 0;
        cardPtr = 0;
        cardPresent = 0;
        key[cardSwitchCol] |= (1u << 4);
        break;
    case 0x50:  /* CRD_READ */
        if (cardMode == 0) {
            bank = (short)((cardBankBuffer[2] & 0x0Fu) / 3);
            if (bank >= 0 && bank < 4) memcpy(cardBankBuffer, cardFullData + bank * 246, 246);
            cardMode = 1;
            cardPtr = 0;
        }
        break;
    case 0xC0:  /* CRD_WRITE */
        if (cardMode == 0) { cardMode = 2; cardPtr = 0; }
        break;
    case 0x60:  /* OUT PRT: one character */
        if (prnReady) {
            code = (u8)((KR >> 4) & 0x3Fu);
            slot = (u8)(prnPtr % 20);
            prnCodeBuf[slot] = code;
            prnPtr = (u8)(slot + 1);
        }
        break;
    case 0x70:  /* OUT PRT_FUNC: a 3-character mnemonic, stored reversed */
        if (prnReady) {
            code = (u8)((KR >> 4) & 0x7Fu);
            for (i = 0; kPrnStr[i].str[0]; i++) {
                if (code == kPrnStr[i].code) {
                    for (k = 3; k > 0; ) {
                        --k;
                        slot = (u8)(prnPtr % 20);
                        prnCodeBuf[slot] = PrnCharToCode(kPrnStr[i].str[k]);
                        prnPtr = (u8)(slot + 1);
                    }
                    break;
                }
            }
        }
        break;
    case 0x80:  /* PRT_CLEAR: arms the printer */
        memset(prnCodeBuf, 0, sizeof(prnCodeBuf));
        prnPtr = 0;
        prnReady = 1;
        break;
    case 0x90:  /* PRT_STEP */
        if (prnReady) {
            slot = (u8)(prnPtr % 20);
            prnCodeBuf[slot] = 0;
            prnPtr = (u8)(slot + 1);
        }
        break;
    case 0xA0:  /* PRT_PRINT; while busy the printer aborts and ignores the new line */
        if (prnReady) {
            if (prnBusyCycles > 0 && prnHasPending) {
                FlushPendingIfActive();
                break;
            }
            for (i = 19; i >= 0; --i) prnPending.codes[19 - i] = prnCodeBuf[i];
            prnHasPending = 1;
            flags |= FLG_BUSY;
            prnBusyCycles = kPrinterLineCycles;
        }
        break;
    case 0xB0:  /* PRT_FEED: half a line of paper */
        {
            TI59PrintLine blank;
            FlushPendingIfActive();
            memset(&blank, 0, sizeof(blank));
            blank.rowCount = 7;
            QueuePrintLine(blank);
        }
        flags |= FLG_BUSY;
        prnBusyCycles = kPrinterFeedCycles;
        break;
    case 0xF0:  /* RAM_OP: the next Sout says what and where */
        flags |= FLG_RAM_OP;
        break;
    default: break;
    }
}

/* ALU class: 0 ffff ooooo ddd - field mask, operation/sources, destination */
void TI59Core::ExecALU(u16 opcode)
{
    const MaskInfo& m = maskInfo[(opcode >> 8) & 0x0Fu];
    const u8* srcX = 0;
    const u8* srcY = 0;
    u8* dst = 0;
    short aluOp = ALU_ADD, constIdx, i;
    u8 masked[16], ioData[16], opNibble, addrTens, addrUnits;
    const u8* constPtr;

    switch ((opcode >> 3) & 0x1Fu) {
    case  0: srcX = A; aluOp = ALU_ADD; break;
    case  1: srcX = A; aluOp = ALU_SUB; break;
    case  2: srcY = B; aluOp = ALU_ADD; break;
    case  3: srcY = B; aluOp = ALU_SUB; break;
    case  4: srcX = C; aluOp = ALU_ADD; break;
    case  5: srcX = C; aluOp = ALU_SUB; break;
    case  6: srcY = D; aluOp = ALU_ADD; break;
    case  7: srcY = D; aluOp = ALU_SUB; break;
    case  8: srcX = A; aluOp = ALU_SHL; break;
    case  9: srcX = A; aluOp = ALU_SHR; break;
    case 10: srcY = B; aluOp = ALU_SHL; break;
    case 11: srcY = B; aluOp = ALU_SHR; break;
    case 12: srcX = C; aluOp = ALU_SHL; break;
    case 13: srcX = C; aluOp = ALU_SHR; break;
    case 14: srcY = D; aluOp = ALU_SHL; break;
    case 15: srcY = D; aluOp = ALU_SHR; break;
    case 16: srcX = A; srcY = B; aluOp = ALU_ADD; break;
    case 17: srcX = A; srcY = B; aluOp = ALU_SUB; break;
    case 18: srcX = C; srcY = B; aluOp = ALU_ADD; break;
    case 19: srcX = C; srcY = B; aluOp = ALU_SUB; break;
    case 20: srcX = C; srcY = D; aluOp = ALU_ADD; break;
    case 21: srcX = C; srcY = D; aluOp = ALU_SUB; break;
    case 22: srcX = A; srcY = D; aluOp = ALU_ADD; break;
    case 23: srcX = A; srcY = D; aluOp = ALU_SUB; break;
    default: break;
    }

    switch (opcode & 0x0007u) {
    case 0: dst = A; break;
    case 1: flags |= FLG_IO_VALID; break;   /* result to Sout only */
    case 3: dst = B; break;
    case 4: dst = C; break;
    case 6: dst = D; break;
    default: break;                         /* 2, 5, 7: exchanges below */
    }

    /* constant ROM row from KR bits 4-6 and 8-11 */
    constIdx = (short)(((KR >> 5) & 0x78u) | ((KR >> 4) & 0x07u));
    if (constIdx >= 64) constIdx = 0;
    constPtr = constant[constIdx];

    switch (opcode & 0x00F8u) {
    case 0x00C0: Alu(dst, A, constPtr, m, ALU_ADD); break;
    case 0x00C8: Alu(dst, A, constPtr, m, ALU_SUB); break;
    case 0x00D0:    /* MOV dst,#0 - or the pending recall from SCOM / RAM */
        if (flags & FLG_RECALL) {
            flags &= ~FLG_RECALL;
            ReadScomMasked(masked, REG_ADDR, m);
            Alu(dst, 0, masked, m, ALU_ADD);
        } else if ((flags & FLG_RAM_READ) && RAM_ADDR < kRamRegs) {
            flags &= ~FLG_RAM_READ;
            ReadRegMasked(masked, RAM_ADDR, m);
            Alu(dst, 0, masked, m, ALU_ADD);
        } else {
            Alu(dst, 0, 0, m, ALU_ADD);
        }
        break;
    case 0x00D8: Alu(dst, 0, 0, m, ALU_SUB); break;
    case 0x00E0: Alu(dst, C, constPtr, m, ALU_ADD); break;
    case 0x00E8: Alu(dst, C, constPtr, m, ALU_SUB); break;
    case 0x00F0:    /* MOV dst,R5 (0xF8: the subtracting variant) */
    case 0x00F8:
        if (dst) {
            for (i = (short)(m.start + 1); i <= m.end; i++) dst[i] = 0;
            dst[m.cpos] = m.cval;
            dst[m.start] = R5;
            Alu(dst, 0, dst, m, (opcode & 0x0008u) ? ALU_SUB : ALU_ADD);
        }
        break;
    default:
        Alu(dst, srcX, srcY, m, aluOp);
        break;
    }

    switch (opcode & 0x0007u) {
    case 2: Xch(A, B, m); break;
    case 5: Xch(C, D, m); break;
    case 7: Xch(A, E, m); break;
    default: break;
    }

    /* a pending RAM read/write takes this instruction's field mask */
    if ((flags & FLG_RAM_WRITE) || (flags & FLG_RAM_READ)) RAM_MASK = m;

    /* deferred SCOM store: Sout if this instruction drove the bus, else zero */
    if (flags & FLG_STORE) {
        flags &= ~FLG_STORE;
        memset(ioData, 0, 16);
        if (flags & FLG_IO_VALID) memcpy(ioData, Sout, 16);
        memcpy(SCOM[REG_ADDR], ioData, 16);
    }

    /* deferred RAM operation: Sout[0] = op, Sout[3]*10 + Sout[2] = register */
    if (flags & FLG_RAM_OP) {
        flags &= ~FLG_RAM_OP;
        opNibble = addrTens = addrUnits = 0;
        if (flags & FLG_IO_VALID) {
            opNibble = (u8)(Sout[0] & 0x0Fu);
            addrTens = Sout[3];
            addrUnits = Sout[2];
        }
        RAM_OP = opNibble;
        RAM_ADDR = (u8)((addrTens * 10u) + addrUnits);
        if (RAM_ADDR < kRamRegs) {
            if      (RAM_OP == 2) ClearRamRegs(RAM_ADDR, 1);
            else if (RAM_OP == 4) ClearRamRegs(RAM_ADDR, 10);
            else if (RAM_OP == 1) flags |= FLG_RAM_WRITE;
            else if (RAM_OP == 0) flags |= FLG_RAM_READ;
        }
    } else if (flags & FLG_RAM_WRITE) {
        flags &= ~FLG_RAM_WRITE;
        if (RAM_ADDR < kRamRegs) {
            memset(ioData, 0, 16);
            if (flags & FLG_IO_VALID) memcpy(ioData, Sout, 16);
            WriteRegMasked(RAM_ADDR, ioData, RAM_MASK);
        }
    }

    if (!(flags & FLG_IO_VALID)) memset(Sout, 0, sizeof(Sout));
}

/* Display multiplex: during IDLE, each step strobes one position (digit counter 13..2 = index 11..0),
   with the zero-suppression chain deciding per strobe whether it lights. */
void TI59Core::PostOperation()
{
    u8 strobeDigit, a, b, zero, suppressed, emitsGlyph;
    short idx, dpIdx;
    u32 w;

    strobeDigit = (u8)(digit ? digit - 1 : 15);
    if ((flags & FLG_IDLE) && strobeDigit >= 2 && strobeDigit <= 13) {
        idx = (short)(strobeDigit - 2);
        a = (u8)(A[strobeDigit] & 0x0F);
        b = (u8)(B[strobeDigit] & 0x0F);

        if (idx == 11) zeroSuppressRunning = 1;
        zero = zeroSuppressRunning;
        dpIdx = (R5 >= 2 && R5 <= 13) ? (short)(R5 - 2) : (short)-1;
        if (idx == 1 || dpIdx == idx || b >= 8) zero = 0;
        if (idx == 0) zero = 1;

        suppressed = (u8)(b == 7 || b == 3 || (b <= 4 && zero && a == 0));
        if (suppressed) suppressedMask |= (u16)(1u << idx);
        else            suppressedMask &= (u16)~(1u << idx);

        if (!suppressed && b <= 1 && a != 0) zero = 0;
        zeroSuppressRunning = zero;

        w = StepWeight();
        emitsGlyph = (u8)(b == 0 || b == 1 || b == 5 || b == 6 || b == 8 || b == 9);
        if (!suppressed && emitsGlyph) {
            segA[idx] = a;
            segB[idx] = b;
            litTicks[idx] += w;
        }
        if (R5 == strobeDigit) dpTicks[idx] += w;
    }
}

/* -- Printer ----------------------------------------------------------------- */

void TI59Core::QueuePrintLine(const TI59PrintLine& line)
{
    if (prnCount == kPrnQueue) { prnHead = (short)((prnHead + 1) % kPrnQueue); prnCount--; }
    prnQueue[(prnHead + prnCount) % kPrnQueue] = line;
    prnCount++;
}

u8 TI59Core::TakePrintLine(TI59PrintLine* out)
{
    if (prnCount == 0) return 0;
    *out = prnQueue[prnHead];
    prnHead = (short)((prnHead + 1) % kPrnQueue);
    prnCount--;
    return 1;
}

/* commit the pending line, cut `elapsed` cycles after PRT.GO */
void TI59Core::FlushPendingCodeLine(u32 elapsed)
{
    u32 rows, cols = 0, heat = 0, into, fired;
    if (!prnHasPending) return;
    rows = elapsed / kPrinterRowCycles;
    into = elapsed % kPrinterRowCycles;
    if (rows < 7 && into >= kPrinterRowStartCycles) {
        fired = into - kPrinterRowStartCycles;
        cols = fired / kPrinterColumnCycles + 1;
        if (cols > 5) cols = 5;
        heat = fired - (cols - 1) * kPrinterColumnCycles;
    }
    prnPending.rowCount = (u8)(rows < 7 ? rows : 7);
    prnPending.partialColumns = (u8)cols;
    prnPending.partialHeat = (u8)(heat < 255 ? heat : 255);
    QueuePrintLine(prnPending);
    prnHasPending = 0;
}

void TI59Core::FlushPendingIfActive()
{
    if (!prnHasPending) return;
    FlushPendingCodeLine(kPrinterLineCycles - prnBusyCycles);
}

/* the printer-present sense line is KP at D0; attaching or detaching releases the TRACE latch */
void TI59Core::SetPrinterConnected(u8 connected)
{
    printerConnected = connected;
    printerTrace = 0;
    if (connected) key[0] |= (1u << 2); else key[0] &= ~(1u << 2);
    key[15] &= ~(1u << 2);
}

void TI59Core::PressPrinterPrint(u8 pressed)
{
    if (pressed) key[12] |= (1u << 2); else key[12] &= ~(1u << 2);
}

void TI59Core::PressPrinterAdv(u8 pressed)
{
    if (pressed) key[12] |= (1u << 0); else key[12] &= ~(1u << 0);
}

void TI59Core::SetPrinterTrace(u8 enabled)
{
    printerTrace = enabled;
    if (enabled) key[15] |= (1u << 2); else key[15] &= ~(1u << 2);
}

/* -- Magnetic card ----------------------------------------------------------- */

void TI59Core::InsertCard(const u8* data, u32 count)
{
    memset(cardFullData, 0, sizeof(cardFullData));
    if (data && count > 0) {
        if (count >= 984)      memcpy(cardFullData, data, 984);
        else                   memcpy(cardFullData, data, (size_t)count);   /* one bank: slot 0 */
    }
    memset(cardBankBuffer, 0, sizeof(cardBankBuffer));
    cardPtr = 0;
    cardMode = 0;
    lastWrittenBank = -1;
    cardPresent = 1;
    waitingForCard = 0;
    key[cardSwitchCol] &= ~(1u << 4);
}

const u8* TI59Core::CardEject()
{
    cardPresent = 0;
    waitingForCard = 0;
    key[cardSwitchCol] |= (1u << 4);
    cardMode = 0;
    if (lastWrittenBank < 0 || lastWrittenBank > 3) return 0;
    memcpy(cardOut, cardFullData + lastWrittenBank * 246, 246);
    lastWrittenBank = -1;
    return cardOut;
}
