/* TI59Core.h - the TI-59 from its ROM, for Symantec C++ 7 on the classic Mac.

   The TMC0501 CPU and the TI59Machine of Calc-U-59 (upstream/Core), cut down to what a TI-59 needs:
   CPU, ROM, RAM, constant ROM, keyboard, display, card reader and printer interface (both without the
   host-side queues), library module. Left out: the TI-58/58C variants, tracing, breakpoints, the debug
   overlay, the step-boundary snapshot, threads and locks.

   Written in the C++ Symantec C++ 7 accepts: no STL, no bool, no nullptr, no in-class initialisers,
   no 64-bit types. int is 16 bits there, so every width is spelled out (u8/u16/u32, short, long).

   Accepted when host-ref/reference.txt comes out of port/portrun line for line, PC fingerprint and
   step count included.

   Port (c) 2026 Pit Förster, written with Claude Code via AppleBridge.
   Derived from Calc-U-59, (c) 2026 Martin Erzberger, GPL-3.0 (LICENSE, NOTICE). */

#ifndef TI59CORE_H
#define TI59CORE_H

typedef unsigned char  u8;
typedef unsigned short u16;
#if defined(__LP64__) || defined(_LP64)
typedef unsigned int   u32;
#else
typedef unsigned long  u32;
#endif

/* The ROM image as port/ loads it: 6144 words big-endian, then 64 x 16 constant nibbles
   (host-ref/mkblob.cpp writes it). */
enum { kTI59RomWords = 6144, kTI59ConstBytes = 1024, kTI59BlobBytes = 6144 * 2 + 1024 };

/* Instructions per second in active mode: 455 kHz / 2 / 16. IDLE steps weigh 4. */
#define kTI59InstructionHz 14218.75

/* The keyboard scan loop the ROM idles in (TI-59, no printer). */
enum { kTI59ScanLoLo = 0x063C, kTI59ScanLoHi = 0x0660 };

/* Display state integrated since the previous TakeDisplay(): glyph and control nibble of each position's
   last driven strobe, and how many weighted cycles it was driven (litTicks == 0: dark). Index 0..11 =
   register digit 2..13, so index 11 is the leftmost position. */
struct TI59Display {
    u8  digits[12];
    u8  ctrl[12];
    u32 litTicks[12];
    u32 dpTicks[12];
    u16 suppressedMask;
    u32 pollTicks;      /* weighted cycles the interval covered */
    u32 cTicks;         /* steps the C indicator was driven */
};

/* One printed line: 20 6-bit character codes, left to right; rowCount 7 = complete. */
struct TI59PrintLine {
    u8 codes[20];
    u8 rowCount;
    u8 partialColumns;
    u8 partialHeat;
    u8 pad;             /* an even size: Symantec C++ 7 lays out an array of the 23-byte struct
                           short, its last element overlapped the members after prnQueue */
};

class TI59Core {
public:
    /* step() result bits besides the weight in the low byte */
    enum { kWeightMask = 0xFF };
    /* a backward branch retrying a KEY scan / TST BUSY poll: the ROM is spinning on the keyboard */
    static const u32 kPolledHardware;

    TI59Core();

    /* blob: kTI59BlobBytes bytes as described above */
    void LoadBlob(const u8* blob);
    void LoadLibrary(const u8* data, u32 count);

    /* The state upstream's TI59Machine constructor leaves (no reset() there): what a power-on is. */
    void PowerOn();
    /* TMC0501::reset() + TI59Machine::reset() */
    void Reset();

    u32  Step();
    u16  PC() const { return addr; }

    /* by hardware coordinates (K-line bit, digit slot) and by matrix code row*10+col */
    void PressKey(short row, short col);
    void ReleaseKey(short row, short col);
    void PressMatrix(short code);
    void ReleaseMatrix(short code);

    void SetPrinterConnected(u8 connected);
    void PressPrinterPrint(u8 pressed);
    void PressPrinterAdv(u8 pressed);
    void SetPrinterTrace(u8 enabled);
    /* takes the oldest printed line; 0 when none is waiting */
    u8   TakePrintLine(TI59PrintLine* out);

    void TakeDisplay(TI59Display* out);

    /* Magnetic card: data 0 = blank card to write; CardEject returns the bank written or 0 */
    void InsertCard(const u8* data, u32 count);
    const u8* CardEject();
    u8   WaitingForCard() const { return waitingForCard; }
    u8   CardPresent() const { return cardPresent; }
    short CardMode() const { return cardMode; }       /* 0 none, 1 reading, 2 writing */

    u8   ScomNibble(short row, short col) const { return SCOM[row][col]; }
    const u8* RamReg(short reg) const { return ram[reg]; }

private:
    struct MaskInfo { u8 start, end, cpos, cval; };
    static const MaskInfo maskInfo[16];
    enum { ALU_ADD = 0, ALU_SHL = 1, ALU_SUB = 2, ALU_SHR = 3 };
    enum { kRamRegs = 120, kPrnQueue = 16 };

    /* ROM, RAM, constants */
    u16 rom[kTI59RomWords];
    u8  ram[kRamRegs][16];
    u8  constant[64][16];

    /* CPU */
    u8  A[16], B[16], C[16], D[16], E[16];
    u8  SCOM[16][16];
    u16 KR, SR, fA, fB, EXT, PREG;
    u8  Sout[16];
    u16 flags;
    u8  R5, digit;
    u16 addr;
    u8  REG_ADDR, RAM_ADDR, RAM_OP;
    MaskInfo RAM_MASK;
    u8  key[16];
    u8  lastStepWasHWPoll;

    /* library module */
    u16 libAddr;
    u8  libAddrReadPos;
    u8  libAddrWasWriting;
    u8  libData[5000];

    /* card reader */
    u8  cardSwitchCol;
    u8  cardPresent, waitingForCard;
    u8  cardFullData[984];
    u8  cardBankBuffer[246];
    u16 cardPtr;
    short cardMode;
    short lastWrittenBank;
    u8  cardOut[246];

    /* printer */
    u8  prnCodeBuf[20];
    u8  prnPtr;
    u8  prnReady;
    u32 prnBusyCycles;
    TI59PrintLine prnPending;
    u8  prnHasPending;
    TI59PrintLine prnQueue[kPrnQueue];
    short prnHead, prnCount;
    u8  printerConnected, printerTrace;

    /* display */
    u8  segA[12], segB[12];
    u32 litTicks[12], dpTicks[12];
    TI59Display lastDisplay;
    u16 suppressedMask;
    u8  zeroSuppressRunning;
    u32 cSteps, pollSteps;

    void Alu(u8* dst, const u8* srcX, const u8* srcY, const MaskInfo& m, short op);
    static void Xch(u8* a, u8* b, const MaskInfo& m);
    void ExecALU(u16 opcode);
    void ExecControl(u16 opcode);
    void ExecPeripheral(u16 opcode);
    void PostOperation();
    void ReadRegMasked(u8* dst, short reg, const MaskInfo& m);
    void WriteRegMasked(short reg, const u8* src, const MaskInfo& m);
    void ReadScomMasked(u8* dst, short reg, const MaskInfo& m);
    void ClearRamRegs(short reg, short count);
    void FlushPendingCodeLine(u32 elapsed);
    void FlushPendingIfActive();
    void QueuePrintLine(const TI59PrintLine& line);
    u32  StepWeight() const { return (flags & 0x0001) ? 4 : 1; }
    void ClearState();
};

#endif
