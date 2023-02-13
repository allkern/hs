#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "log.hpp"

#define ERROR(ec, ...) \
    _hv2_log(error, __VA_ARGS__); \
    std::exit(ec)

struct symbol_t {
    std::string name;

    uint32_t value;
};

#define ET_NONE     0x00
#define ET_REL      0x01
#define ET_EXEC     0x02
#define ET_DYN      0x03
#define ET_CORE     0x04
#define ET_LOOS     0xfe00
#define ET_HIOS     0xfeff
#define ET_LOPROC   0xff00
#define ET_HIPROC   0xffff

#define PT_NULL     0x00000000
#define PT_LOAD     0x00000001
#define PT_DYNAMIC  0x00000002
#define PT_INTERP   0x00000003
#define PT_NOTE     0x00000004
#define PT_SHLIB    0x00000005
#define PT_PHDR     0x00000006
#define PT_TLS      0x00000007
#define PT_LOOS     0x60000000
#define PT_HIOS     0x6fffffff
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff

#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4
#define PF_MASKOS   0x0ff00000
#define PF_MASKPROC 0xf0000000

// Can change osabi, abiversion, type, entry, shoff,
// flags?, phentsize, phnum, shentsize, shnum, shstrndx
struct elf32_hdr_t {
    uint8_t ei_mag[4]       = { '\x7f', 'E', 'L', 'F' };
    uint8_t ei_class        = 1; // 32-bit
    uint8_t ei_data         = 1; // LE
    uint8_t ei_version      = 1;
    uint8_t ei_osabi        = 0;
    uint8_t ei_abiversion   = 0;
    uint8_t ei_pad[7]       = { 0 };
    uint16_t e_type         = 0;
    uint16_t e_machine      = 0x1332; // HV2
    uint32_t e_version      = 1;
    uint32_t e_entry        = 0;
    uint32_t e_phoff        = 0x34;
    uint32_t e_shoff        = 0;
    uint32_t e_flags        = 0;
    uint16_t e_ehsize       = sizeof(elf32_hdr_t);
    uint16_t e_phentsize    = 0;
    uint16_t e_phnum        = 0;
    uint16_t e_shentsize    = 0;
    uint16_t e_shnum        = 0;
    uint16_t e_shstrndx     = 0;
};

struct elf32_shdr_t {
	uint32_t sh_name        = 0;
	uint32_t sh_type        = 0;
	uint32_t sh_flags       = 0;
	uint32_t sh_addr        = 0;
	uint32_t sh_offset      = 0;
	uint32_t sh_size        = 0;
	uint32_t sh_link        = 0;
	uint32_t sh_info        = 0;
	uint32_t sh_addralign   = 0;
	uint32_t sh_entsize     = 0;
};

struct elf32_phdr_t {
	uint32_t p_type         = 0;
	uint32_t p_offset       = 0;
	uint32_t p_vaddr        = 0;
	uint32_t p_paddr        = 0;
	uint32_t p_filesz       = 0;
	uint32_t p_memsz        = 0;
	uint32_t p_flags        = 0;
	uint32_t p_align        = 0;
};

std::unordered_map <std::string, uint32_t> register_names = {
    // Registers and aliases
    { "r0", 0 }, { "zero", 0 },
    { "r1", 1 }, { "at", 1 },
    { "r2", 2 }, { "a0", 2 },
    { "r3", 3 }, { "x0", 3 },
    { "r4", 4 }, { "x1", 4 },
    { "r5", 5 }, { "x2", 5 },
    { "r6", 6 }, { "x3", 6 },
    { "r7", 7 }, { "x4", 7 },
    { "r8", 8 }, { "x5", 8 },
    { "r9", 9 }, { "x6", 9 },
    { "r10", 10 }, { "x7", 10 },
    { "r11", 11 }, { "x8", 11 },
    { "r12", 12 }, { "x9", 12 },
    { "r13", 13 }, { "x10", 13 },
    { "r14", 14 }, { "x11", 14 },
    { "r15", 15 }, { "x12", 15 },
    { "r16", 16 }, { "x13", 16 },
    { "r17", 17 }, { "x14", 17 },
    { "r18", 18 }, { "x15", 18 },
    { "r19", 19 }, { "x16", 19 },
    { "r20", 20 }, { "x17", 20 },
    { "r21", 21 }, { "x18", 21 },
    { "r22", 22 }, { "x19", 22 },
    { "r23", 23 }, { "x20", 23 },
    { "r24", 24 }, { "x21", 24 },
    { "r25", 25 }, { "x22", 25 },
    { "r26", 26 }, { "x23", 26 },
    { "r27", 27 }, { "x24", 27 },
    { "r28", 28 }, { "fp", 28 },
    { "r29", 29 }, { "sp", 29 },
    { "r30", 30 }, { "lr", 30 },
    { "r31", 31 }, { "pc", 31 },

    // COP0 registers
    { "cop0_cr0", 0x000 },
    { "cop0_cr1", 0x010 },
    { "cop0_xcause", 0x020 },
    { "cop0_xhaddr", 0x030 },
    { "cop0_xpc", 0x040 }
};

struct elf_section_t {
    std::string name;

    uint32_t shstrtab_off;

    elf32_shdr_t hdr;
};

struct hv2a_t {
    std::iostream* output;
    uint32_t pos = 0;
    uint32_t vaddr = 0;
    uint32_t entry;

    std::vector <elf_section_t> sections = {
        { "", { 0 } }
    };

    std::unordered_map <std::string, uint32_t> global_symbols;
    std::unordered_map <std::string, uint32_t> local_symbols;
    std::string current_symbol = "none";

    bool flush = false;
    unsigned pipeline_size = 3;

    uint32_t output_format;

    // Parser
    std::istream* stream;
    int pass = 0;
    char c;
};

#define E_OK                    0x00000000
#define E_UNDEFINED_SYMBOL      0x40000001
#define E_INVALID_MODE          0x40000002
#define E_UNKNOWN_MNEMONIC      0x40000003
#define E_UNREACHABLE           0x40000004
#define E_EXP_INT_OR_SYMBOL     0xc0000000
#define E_EXP_MNEMONIC_OR_LABEL 0xc0000001
#define E_EXP_COMMA             0xc0000002
#define E_EXP_IDX_END_OR_OP     0xc0000003
#define E_EXP_IDX_END           0xc0000004
#define E_EXP_INTEGER_LITERAL   0xc0000005

#include <string>

struct mnemonic_data_t {
    std::string mnemonic;
    uint32_t type;
    uint32_t alu_op;
    bool     alu_sign;
    bool     alu_mode;
    bool     brn_link;
    uint32_t brr_cond;
    uint32_t cpe_op;
    uint32_t sys_op;
    uint32_t lsl_op;
    uint32_t lsl_size;
    bool     sci_sx;
    bool     li_sx;
    uint32_t pseudo_op;
    uint32_t pseudo_data[2];
};

#define ALU(o, s, m)     { "", 0x00, o, s, m, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define BRN(t, l)        { "", t   , 0, 0, 0, l, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define BRR(l, c)        { "", 0x0d, 0, 0, 0, l, c, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define CPE(o)           { "", 0x0e, 0, 0, 0, 0, 0, o, 0, 0, 0, 0, 0, 0, 0, 0 }
#define CPI()            { "", 0x1e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define SCI(t, s)        { "", t   , 0, 0, 0, 0, 0, 0, 0, 0, 0, s, 0, 0, 0, 0 }
#define SYS(o)           { "", 0x0f, 0, 0, 0, 0, 0, 0, o, 0, 0, 0, 0, 0, 0, 0 }
#define LSL(o, s)        { "", 0x10, 0, 0, 0, 0, 0, 0, 0, o, s, 0, 0, 0, 0, 0 }
#define LI(s)            { "", 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, s, 0, 0, 0 }
#define PSD0(o)          { "", 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, o, 0, 0 }
#define PSD1(o, a)       { "", 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, o, a, 0 }
#define PSD2(o, a, b)    { "", 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, o, a, b }

std::unordered_map <std::string, mnemonic_data_t> mnemonic_data_map = {
    // ALU register/immediate
    { "add",  ALU(0x0, 1, 0) }, { "add.u",  ALU(0x0, 0, 0) }, { "add.s",  ALU(0x0, 1, 0) },
    { "addi", ALU(0x0, 1, 1) }, { "addi.u", ALU(0x0, 0, 1) }, { "addi.s", ALU(0x0, 1, 1) },
    { "sub",  ALU(0x1, 1, 0) }, { "sub.u",  ALU(0x1, 0, 0) }, { "sub.s",  ALU(0x1, 1, 0) },
    { "subi", ALU(0x1, 1, 1) }, { "subi.u", ALU(0x1, 0, 1) }, { "subi.s", ALU(0x1, 1, 1) },
    { "mul",  ALU(0x2, 1, 0) }, { "mul.u",  ALU(0x2, 0, 0) }, { "mul.s",  ALU(0x2, 1, 0) },
    { "muli", ALU(0x2, 1, 1) }, { "muli.u", ALU(0x2, 0, 1) }, { "muli.s", ALU(0x2, 1, 1) },
    { "mla",  ALU(0x3, 1, 0) }, { "mla.u",  ALU(0x3, 0, 0) }, { "mla.s",  ALU(0x3, 1, 0) },
    { "mlai", ALU(0x3, 1, 1) }, { "mlai.u", ALU(0x3, 0, 1) }, { "mlai.s", ALU(0x3, 1, 1) },
    { "div",  ALU(0x4, 1, 0) }, { "div.u",  ALU(0x4, 0, 0) }, { "div.s",  ALU(0x4, 1, 0) },
    { "divi", ALU(0x4, 1, 1) }, { "divi.u", ALU(0x4, 0, 1) }, { "divi.s", ALU(0x4, 1, 1) },
    { "mod",  ALU(0x5, 1, 0) }, { "mod.u",  ALU(0x5, 0, 0) }, { "mod.s",  ALU(0x5, 1, 0) },
    { "modi", ALU(0x5, 1, 1) }, { "modi.u", ALU(0x5, 0, 1) }, { "modi.s", ALU(0x5, 1, 1) },
    { "and",  ALU(0x6, 1, 0) }, { "and.u",  ALU(0x6, 0, 0) }, { "and.s",  ALU(0x6, 1, 0) },
    { "andi", ALU(0x6, 1, 1) }, { "andi.u", ALU(0x6, 0, 1) }, { "andi.s", ALU(0x6, 1, 1) },
    { "or",   ALU(0x7, 1, 0) }, { "or.u",   ALU(0x7, 0, 0) }, { "or.s",   ALU(0x7, 1, 0) },
    { "ori",  ALU(0x7, 1, 1) }, { "ori.u",  ALU(0x7, 0, 1) }, { "ori.s",  ALU(0x7, 1, 1) },
    { "xor",  ALU(0x8, 1, 0) }, { "xor.u",  ALU(0x8, 0, 0) }, { "xor.s",  ALU(0x8, 1, 0) },
    { "xori", ALU(0x8, 1, 1) }, { "xori.u", ALU(0x8, 0, 1) }, { "xori.s", ALU(0x8, 1, 1) },
    { "lsl",  ALU(0x9, 1, 0) }, { "lsl.u",  ALU(0x9, 0, 0) }, { "lsl.s",  ALU(0x9, 1, 0) },
    { "lsli", ALU(0x9, 1, 1) }, { "lsli.u", ALU(0x9, 0, 1) }, { "lsli.s", ALU(0x9, 1, 1) },
    { "lsr",  ALU(0xa, 1, 0) }, { "lsr.u",  ALU(0xa, 0, 0) }, { "lsr.s",  ALU(0xa, 1, 0) },
    { "lsri", ALU(0xa, 1, 1) }, { "lsri.u", ALU(0xa, 0, 1) }, { "lsri.s", ALU(0xa, 1, 1) },
    { "asr",  ALU(0xb, 1, 0) }, { "asr.u",  ALU(0xb, 0, 0) }, { "asr.s",  ALU(0xb, 1, 0) },
    { "asri", ALU(0xb, 1, 1) }, { "asri.u", ALU(0xb, 0, 1) }, { "asri.s", ALU(0xb, 1, 1) },
    { "sx.b", ALU(0xc, 1, 1) }, { "sx.s",   ALU(0xd, 1, 1) },
    { "rol",  ALU(0xe, 1, 0) }, { "rol.u",  ALU(0xe, 0, 0) }, { "rol.s",  ALU(0xe, 1, 0) },
    { "roli", ALU(0xe, 1, 1) }, { "roli.u", ALU(0xe, 0, 1) }, { "roli.s", ALU(0xe, 1, 1) },
    { "ror",  ALU(0xf, 1, 0) }, { "ror.u",  ALU(0xf, 0, 0) }, { "ror.s",  ALU(0xf, 1, 0) },
    { "rori", ALU(0xf, 1, 1) }, { "rori.u", ALU(0xf, 0, 1) }, { "rori.s", ALU(0xf, 1, 1) },

    // Branch immediate
    { "beq",  BRN(0x02, 0) }, { "bleq", BRN(0x02, 1) },
    { "bne",  BRN(0x04, 0) }, { "blne", BRN(0x04, 1) },
    { "bgt",  BRN(0x06, 0) }, { "blgt", BRN(0x06, 1) },
    { "bge",  BRN(0x08, 0) }, { "blge", BRN(0x08, 1) },
    { "blt",  BRN(0x0a, 0) }, { "bllt", BRN(0x0a, 1) },
    { "ble",  BRN(0x0c, 0) }, { "blle", BRN(0x0c, 1) },

    // Branch register
    { "beq.r", BRR(0, 1) }, { "bleq.r", BRR(1, 1) },
    { "bne.r", BRR(0, 2) }, { "blne.r", BRR(1, 2) },
    { "bgt.r", BRR(0, 3) }, { "blgt.r", BRR(1, 3) },
    { "bge.r", BRR(0, 4) }, { "blge.r", BRR(1, 4) },
    { "blt.r", BRR(0, 5) }, { "bllt.r", BRR(1, 5) },
    { "ble.r", BRR(0, 6) }, { "blle.r", BRR(1, 6) },

    // COP-CPU exchange
    { "mtcr", CPE(0) },
    { "mfcr", CPE(1) },

    // COP instruction
    { "cpex", CPI() },

    // Set Cond Immediate
    { "seq.u", SCI(0x13, 0) }, { "seq.s", SCI(0x13, 1) },
    { "sne.u", SCI(0x15, 0) }, { "sne.s", SCI(0x15, 1) },
    { "sgt.u", SCI(0x17, 0) }, { "sgt.s", SCI(0x17, 1) },
    { "sge.u", SCI(0x19, 0) }, { "sge.s", SCI(0x19, 1) },
    { "slt.u", SCI(0x1b, 0) }, { "slt.s", SCI(0x1b, 1) },
    { "sle.u", SCI(0x1d, 0) }, { "sle.s", SCI(0x1d, 1) },

    // System
    { "syscall", SYS(0) },
    { "tpl0",    SYS(1) },
    { "tpl1",    SYS(2) },
    { "tpl2",    SYS(3) },
    { "tpl3",    SYS(4) },
    { "debug",   SYS(5) },
    { "excep",   SYS(6) },

    // Load/Store/LEA
    { "load.b",  LSL(0, 0) }, { "load.s",  LSL(0, 1) }, { "load.l",  LSL(0, 2) }, { "load.x",  LSL(0, 3) },
    { "store.b", LSL(1, 0) }, { "store.s", LSL(1, 1) }, { "store.l", LSL(1, 2) }, { "store.x", LSL(1, 3) },
    { "lea.b",   LSL(2, 0) }, { "lea.s",   LSL(2, 1) }, { "lea.l",   LSL(2, 2) }, { "lea.x",   LSL(2, 3) },

    // Load immediate
    { "li", LI(1) }, { "li.u", LI(0) }, { "li.s", LI(1) }
};

#define PSD_B       1
#define PSD_LIW     2
#define PSD_NOP     3
#define PSD_MOVE    4
#define PSD_NOT     5
#define PSD_SWAP    6
#define PSD_CLR     7
#define PSD_ZX      8
#define PSD_INC     9
#define PSD_DEC     10
#define PSD_CALLI   11
#define PSD_CALLR   12
#define PSD_RET     13
#define PSD_PUSH    14
#define PSD_POP     15
#define PSD_BI      16
#define PSD_XCH     17

std::unordered_map <std::string, mnemonic_data_t> pseudo_data_map = {
    // Branch always
    { "b", PSD0(PSD_B) }, { "bra", PSD0(PSD_B) },
    
    // Load word
    { "li.w", PSD0(PSD_LIW) },

    // nop
    { "nop", PSD0(PSD_NOP) },

    // move
    { "move", PSD0(PSD_MOVE) }, { "mov", PSD0(PSD_MOVE) },

    // not
    { "not", PSD0(PSD_NOT) },

    // swap
    { "swap", PSD1(PSD_SWAP, 0) },

    // clr
    { "clr", PSD0(PSD_CLR) },

    // Zero-extend
    { "zx.b", PSD1(PSD_ZX, 0) }, { "zx.s", PSD1(PSD_ZX, 1) },

    // inc/dec
    { "inc", PSD0(PSD_INC) }, { "dec", PSD0(PSD_INC) },

    // call.i/r/ret
    { "call.i", PSD0(PSD_CALLI), }, { "call", PSD0(PSD_CALLI), }, { "call.r", PSD0(PSD_CALLR), },
    { "ret", PSD0(PSD_RET) },

    // push/pop
    { "push", PSD0(PSD_PUSH) }, { "pop", PSD0(PSD_POP) },

    // Branch immediate if cond immediate
    { "beq.i" , PSD2(PSD_BI, 0, 1) }, { "bne.i" , PSD2(PSD_BI, 0, 2) },
    { "bgt.i" , PSD2(PSD_BI, 0, 3) }, { "bge.i" , PSD2(PSD_BI, 0, 4) },
    { "blt.i" , PSD2(PSD_BI, 0, 5) }, { "ble.i" , PSD2(PSD_BI, 0, 6) },
    { "bleq.i", PSD2(PSD_BI, 1, 1) }, { "blne.i", PSD2(PSD_BI, 1, 2) },
    { "blgt.i", PSD2(PSD_BI, 1, 3) }, { "blge.i", PSD2(PSD_BI, 1, 4) },
    { "bllt.i", PSD2(PSD_BI, 1, 5) }, { "blle.i", PSD2(PSD_BI, 1, 6) },

    // Exchange registers
    { "xch", PSD0(PSD_XCH) }
};

#undef ALU
#undef BRN
#undef BRR
#undef CPE
#undef CPI
#undef SCI
#undef SYS
#undef LSL
#undef LI

/*
Encodings summary:
ALU register:               iiiiixxx xxyyyyyz zzzzz--- --OOOOMS
ALU immediate:              iiiiixxx xxIIIIII IIIIIIII IIOOOOMS
Branch register:            01101xxx xxyyyyyz zzzzwwww wIIIcccM
Branch immediate:           Sccc0xxx xxyyyyyI IIIIIIII IIIIIIIL
Coprocessor-CPU exchange:   01110xxx xxyyyyyy yyyycccc c--OOOOO
Coprocessor instruction:    1111iiii iiiiiiii iiiiiiii iiiicccc
System:                     01111ooo cccccccc cccccccc cccccccc
Load/Store/LEA Fixed:       iiiiixxx xxyyyyyI IIIIIIII ISSOOmmm
Load/Store/LEA Register:    iiiiixxx xxyyyyyz zzzzwwww wSSOOmmm
Load immediate:             10001xxx xxIIIIII IIIIIIII IISsssss
Set Cond Immediate:         1ccc1xxx xxyyyyyI IIIIIIII IIIIIIIS
*/

// Operand modes
#define OPR_NONE    -1
#define OPR_INT1    0
#define OPR_INT2    1
#define OPR_INT3    2
#define OPR_IDX_INT 3
#define OPR_IDX1    4

std::string operand_mode_name[] = {
    "OPR_INT1",
    "OPR_INT2",
    "OPR_INT3",
    "OPR_IDX_INT",
    "OPR_IDX1"
};

// Operand types
#define OPR_INT   0
#define OPR_IDX   1

inline uint32_t hv2a_get_pipeline_offset(hv2a_t* as) {
    if (as->flush)
        return 0;
    
    return as->pipeline_size * 4;
}

inline void hv2a_consume_whitespace(hv2a_t* as) {
    while (std::isblank(as->c))
        as->c = as->stream->get();
}

inline bool is_integer_or_symbol(char c) {
    return std::isalnum(c) || (c == '\'') || (c == '-') || (c == '!') || (c == '_');
}

inline bool is_index(char c) {
    return c == '[';
}

inline bool is_string(char c) {
    return c == '\"';
}

uint32_t hv2a_get_operand_type(hv2a_t* as) {
    if (is_integer_or_symbol(as->c)) return OPR_INT;
    if (as->c == '[') return OPR_IDX;

    return OPR_NONE;
}

#define IT_ALU          0b00000
#define IT_BEQ          0b00010
#define IT_BNE          0b00100
#define IT_BGT          0b00110
#define IT_BGE          0b01000
#define IT_BLT          0b01010
#define IT_BLE          0b01100
#define IT_BRR          0b01101
#define IT_CPE          0b01110
#define IT_SYS          0b01111
#define IT_LSL          0b10000
#define IT_LI           0b10001
//#define IT_BEQ1        0b10010
//#define IT_BNE1        0b10100
//#define IT_BGT1        0b10110
//#define IT_BGE1        0b11000
//#define IT_BLT1        0b11010
//#define IT_BLE1        0b11100
#define IT_CPI          0b11110
//#define IT_CPI1         0b11111
#define IT_SEQ          0b10011
#define IT_SNE          0b10101
#define IT_SGT          0b10111
#define IT_SGE          0b11001
#define IT_SLT          0b11011
#define IT_SLE          0b11101

struct operand_data_t {
    uint32_t mode = OPR_NONE;
    uint32_t integer[3] = { 0 };
    uint32_t idx_base = 0, idx_index = 0, idx_scale = 0;
    uint32_t idx_fix = 0;
    bool fixed = false, add = false, mult = false;
};

// Line:
// label: label: ... label: mnemonic {operands}

#include <string>
#include <sstream>

symbol_t hv2a_symbol_lookup(hv2a_t* as, std::string name) {
    if (as->global_symbols.contains(name)) {
        return { name, as->global_symbols[name] };
    }

    std::string local = name + as->current_symbol;

    if (as->local_symbols.contains(local)) {
        return { name, as->local_symbols[local] };
    }
    
    return { "", 0 };
}

#define INT_TYPE_LITERAL  0 
#define INT_TYPE_SYMBOL   1
#define INT_TYPE_REGISTER 2

std::string hv2a_parse_string(hv2a_t* as) {
    if (!is_string(as->c)) {
        ERROR(E_EXP_INT_OR_SYMBOL, "Expected string, got \'%c\' instead", as->c);
    }

    std::string buf;

    as->c = as->stream->get();

    while (as->c != '\"') {
        if (as->c == '\\') {
            as->c = as->stream->get();

            switch (as->c) {
                case 'a' : { buf.push_back('\x07'); } break; // Alert (Beep, Bell)
                case 'b' : { buf.push_back('\x08'); } break; // Backspace
                case 'e' : { buf.push_back('\x1b'); } break; // Escape character
                case 'f' : { buf.push_back('\x0c'); } break; // Formfeed Page Break
                case 'n' : { buf.push_back('\x0a'); } break; // Newline (Line Feed); see notes below
                case 'r' : { buf.push_back('\x0d'); } break; // Carriage Return
                case 't' : { buf.push_back('\x09'); } break; // Horizontal Tab
                case 'v' : { buf.push_back('\x0b'); } break; // Vertical Tab
                case '\\': { buf.push_back('\x5c'); } break; // Backslash
                case '\'': { buf.push_back('\x27'); } break; // Apostrophe or single quotation mark
                case '\"': { buf.push_back('\x22'); } break; // Double quotation mark
                // To-do: Parse \xNN and \uNNNN style sequences
            }
        } else {
            buf.push_back(as->c);
        }

        as->c = as->stream->get();
    }

    as->c = as->stream->get();

    return buf;
}

uint32_t hv2a_parse_integer(hv2a_t* as, int* type = nullptr) {
    if (!is_integer_or_symbol(as->c)) {
        ERROR(E_EXP_INT_OR_SYMBOL, "Expected integer or symbol, got \'%c\' instead", as->c);
    }

    std::string buf;

    uint32_t v;

    bool absolute = as->c == '!';

    if (absolute)
        as->c = as->stream->get();

    bool negative = as->c == '-';

    if (negative)
        as->c = as->stream->get();

    if (as->c == '\'') {
        as->c = as->stream->get();

        while (as->c != '\'') {
            buf.push_back(as->c);

            as->c = as->stream->get();
        }
    } else {
        while (std::isalnum(as->c) || (as->c == '_')) {
            buf.push_back(as->c);

            as->c = as->stream->get();
        }
    }

    // Integer literal
    if (std::isdigit(buf[0])) {
        switch (buf[1]) {
            case 'x': {
                v = std::stoul(buf, nullptr, 16);
            } break;

            case 'b': {
                for (int i = 0; i < buf.size(); i++) {
                    v >>= 1;
                    v |= (buf[i] == '1') << 31;
                }
            } break;

            default: {
                v = std::stoul(buf, nullptr, 0);
            } break;
        }

        if (type) *type = INT_TYPE_LITERAL;
    }

    // Char literal
    if (buf[0] == '\'') {
        if (buf[1] == '\\') {
            switch (buf[2]) {
                case 'a' : { v = (int)'\x07'; } break; // Alert (Beep, Bell)
                case 'b' : { v = (int)'\x08'; } break; // Backspace
                case 'e' : { v = (int)'\x1b'; } break; // Escape character
                case 'f' : { v = (int)'\x0c'; } break; // Formfeed Page Break
                case 'n' : { v = (int)'\x0a'; } break; // Newline (Line Feed); see notes below
                case 'r' : { v = (int)'\x0d'; } break; // Carriage Return
                case 't' : { v = (int)'\x09'; } break; // Horizontal Tab
                case 'v' : { v = (int)'\x0b'; } break; // Vertical Tab
                case '\\': { v = (int)'\x5c'; } break; // Backslash
                case '\'': { v = (int)'\x27'; } break; // Apostrophe or single quotation mark
                case '\"': { v = (int)'\x22'; } break; // Double quotation mark
                // To-do: Parse \xNN and \uNNNN style sequences
            }
        } else {
            v = (int)buf[1];
        }

        if (type) *type = INT_TYPE_LITERAL;
    }

    // Symbol
    if (std::isalpha(buf[0]) || (buf[0] == '_')) {
        if (as->pass == 0)
            return 0x0;
        
        if (register_names.contains(buf)) {
            uint32_t rn = register_names[buf];

            if (type) *type = INT_TYPE_REGISTER;

            // This is likely VERY useless
            return negative ? -rn : rn;
        }

        symbol_t sym = hv2a_symbol_lookup(as, buf);

        if (!sym.name.size()) {
            ERROR(E_EXP_INT_OR_SYMBOL, "Undefined symbol \"%s\"", buf.c_str());
        }

        if (type) *type = INT_TYPE_SYMBOL;

        // Account for different pipeline sizes
        // Ignore negative marker
        return absolute ? sym.value : (sym.value - (as->vaddr + hv2a_get_pipeline_offset(as)));
    }

    return negative ? -v : v;
}

// [rs0]
// [rs0+rs1]
// [rs0-rs1]
// [rs0+rs1*s2]
// [rs0-rs1*s2]
// [rs0+rs1:s2]
// [rs0-rs1:s2]
// [rs0+i10]
// [rs0-i10]
void hv2a_parse_idx(hv2a_t* as, operand_data_t* od) {
    if (!is_index(as->c)) {
        //ERROR()
    }

    as->c = as->stream->get();

    hv2a_consume_whitespace(as);
    
    od->idx_base = hv2a_parse_integer(as);

    hv2a_consume_whitespace(as);

    switch (as->c) {
        case ']': {
            as->c = as->stream->get();

            return;
        } break;

        case '+': case '-': {
            od->add = as->c == '+';
        } break;

        case ',': {
            od->add = true;
        } break;

        default: {
            ERROR(E_EXP_IDX_END_OR_OP, "Expected \']\' or operator");
        } break;
    }

    as->c = as->stream->get();

    hv2a_consume_whitespace(as);

    int op2_type = 0;

    uint32_t op2 = hv2a_parse_integer(as, &op2_type);

    switch (op2_type) {
        // Fixed
        case INT_TYPE_LITERAL: case INT_TYPE_SYMBOL: {
            od->idx_fix = op2;
            od->fixed = true;

            hv2a_consume_whitespace(as);

            if (as->c != ']') {
                ERROR(E_EXP_IDX_END, "Expected \']\'");
            }

            as->c = as->stream->get();

            return;
        } break;

        // Register
        case INT_TYPE_REGISTER: {
            od->fixed = false;
            od->idx_index = op2;

            hv2a_consume_whitespace(as);

            switch (as->c) {
                case ']': {
                    as->c = as->stream->get();

                    return;
                } break;

                case '*': case ':': {
                    od->mult = as->c == '*';
                } break;

                case ',': {
                    od->mult = true;
                } break;

                default: {
                    ERROR(E_EXP_IDX_END_OR_OP, "Expected \']\' or operator");
                } break;
            }

            as->c = as->stream->get();

            hv2a_consume_whitespace(as);

            int op3_type = 0;

            uint32_t op3 = hv2a_parse_integer(as, &op3_type);

            if ((op3_type != INT_TYPE_LITERAL) && (op3_type != INT_TYPE_SYMBOL)) {
                ERROR(E_EXP_INTEGER_LITERAL, "Expected integer literal");
            }

            od->idx_scale = op3;

            hv2a_consume_whitespace(as);

            if (as->c != ']') {
                ERROR(E_EXP_IDX_END, "Expected \']\'");
            }

            as->c = as->stream->get();
        } break;
    }
}

#include <cmath>

inline int ffs(uint32_t v) {
    return std::log2(v & -v);
}

inline uint32_t encode_d(uint32_t v) { return (v & 0x1f) << 22; }
inline uint32_t encode_s0(uint32_t v) { return (v & 0x1f) << 17; }
inline uint32_t encode_s1(uint32_t v) { return (v & 0x1f) << 12; }
inline uint32_t encode_s2(uint32_t v) { return (v & 0x1f) << 7; }

void hv2a_assemble(hv2a_t*);

uint32_t encode_instruction(mnemonic_data_t* md, operand_data_t* od, size_t* size = nullptr) {
    uint32_t opcode = 0x00000000;

    // Encode instruction type
    opcode |= md->type << 27;

    switch (md->type) {
        case IT_ALU: {
            opcode |= encode_d(od->integer[0]);
            opcode |= md->alu_op << 2;
            opcode |= md->alu_sign;

            // Allow 1 operand for sx.b/s
            if (od->mode == OPR_INT1) {
                if (!((md->alu_op == 12) || (md->alu_op == 13))) {
                    ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
                }

                opcode |= encode_d(od->integer[0]);
                
                break;
            }

            // Immediate mode
            if (od->mode == OPR_INT2) {
                opcode |= 1 << 1;
                opcode |= (od->integer[1] & 0xffff) << 6;

                break;
            }

            if (od->mode == OPR_INT3) {
                opcode |= 0 << 1;
                opcode |= encode_s0(od->integer[1]);
                opcode |= encode_s1(od->integer[2]);

                break;
            }

            ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
        } break;

        case IT_BEQ: case IT_BNE:
        case IT_BGT: case IT_BGE:
        case IT_BLT: case IT_BLE: {
            if (od->mode != OPR_INT3) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            od->integer[2] &= 0x1ffff;

            opcode |= encode_d(od->integer[0]);
            opcode |= encode_s0(od->integer[1]);
            opcode |= (od->integer[2] & 0xffff) << 1;
            opcode |= (od->integer[2] & 0x10000) << 15;
            opcode |= md->brn_link;
        } break;

        case IT_BRR: {
            if (od->mode != OPR_INT3) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            opcode |= encode_d(od->integer[0]);
            opcode |= encode_s0(od->integer[1]);
            opcode |= encode_s1(od->integer[2]);
            opcode |= md->brr_cond << 1;
        } break;

        case IT_CPE: {
            if ((od->mode != OPR_INT3) && (od->mode != OPR_INT2)) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            opcode |= md->cpe_op;

            // mtcr and mfcr have swapped operand order to
            // follow "destination <- source" order convention
            // mtcr copn, copr, cpur
            // mfcr cpur, copn, copr
            // This assembler also accepts a 2 operand form
            // where the coprocessor number and register are
            // merged into a single integer, this is to allow
            // syntax like:
            // mfcr x0, cop0_xpc
            if (md->cpe_op == 0) {
                if (od->mode == OPR_INT3) {
                    opcode |= od->integer[0] << 8;
                    opcode |= (od->integer[1] & 0x3ff) << 12;
                    opcode |= encode_d(od->integer[2]);
                } else if (od->mode == OPR_INT2) {
                    opcode |= encode_d(od->integer[1]);
                    opcode |= (od->integer[0] & 0x3fff) << 8;
                }
            }

            if (md->cpe_op == 1) {
                if (od->mode == OPR_INT3) {
                    opcode |= encode_d(od->integer[0]);
                    opcode |= od->integer[1] << 8;
                    opcode |= (od->integer[2] & 0x3ff) << 12;
                } else if (od->mode == OPR_INT2) {
                    opcode |= encode_d(od->integer[0]);
                    opcode |= (od->integer[1] & 0x3fff) << 8;
                }
            }
        } break;

        case IT_SYS: {
            if (od->mode != OPR_INT1) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            opcode |= md->sys_op << 24;
            opcode |= od->integer[0] & 0xffffff;

            // Special case TPL0-3
            // if ((md->sys_op >= 1) && (md->sys_op <= 4)) {

            // }
        } break;

        case IT_LSL: {
            opcode |= md->lsl_op << 3;
            opcode |= md->lsl_size << 5;

            // Encode mode according to the following
            // table:
            // 000: Add scaled register
            // 001: Sub scaled register
            // 010: Add shifted register
            // 011: Sub shifted register
            // 100: Add fixed (MSB 0)
            // 101: Sub fixed (MSB 0)
            // 110: Add fixed (MSB 1)
            // 111: Sub fixed (MSB 1)
            opcode |= (od->fixed << 2) | (!od->add);

            opcode |= encode_d(od->integer[0]);
            opcode |= encode_s0(od->idx_base);

            if (od->fixed) {
                od->idx_fix &= 0x7ff;

                // Multiply flag is used as fix MSB instead
                opcode |= (od->idx_fix & 0x400) >> 9;
                opcode |= (od->idx_fix & 0x3ff) << 7;
            } else {
                // Encode multiply flag
                opcode |= ((!od->mult) << 1);
                opcode |= encode_s1(od->idx_index);
                opcode |= encode_s2(od->idx_scale);
            }
        } break;

        case IT_LI: {
            if (od->mode != OPR_INT2) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            opcode |= encode_d(od->integer[0]);

            // Find first set bit
            int fsb = ffs(od->integer[1]);

            opcode |= ((od->integer[1] >> fsb) & 0xffff) << 6;
            opcode |= md->li_sx << 5;
            opcode |= fsb;
        } break;
        
        case IT_CPI: {
            if (od->mode != OPR_INT2) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            opcode |= od->integer[0];
            opcode |= (od->integer[1] & 0xffffff) << 4;
        } break;

        case IT_SEQ: case IT_SNE:
        case IT_SGT: case IT_SGE:
        case IT_SLT: case IT_SLE: {
            if (od->mode != OPR_INT3) {
                ERROR(E_INVALID_MODE, "Invalid mode %s for %s", operand_mode_name[od->mode].c_str(), md->mnemonic.c_str());
            }

            opcode |= encode_d(od->integer[0]);
            opcode |= encode_s0(od->integer[1]);
            opcode |= (od->integer[2] & 0xffff) << 1;
            opcode |= md->sci_sx;
        } break;
    }

    return opcode;
}

#define AD_ORG      0x0000
#define AD_DB       0x0001
#define AD_DS       0x0002
#define AD_DL       0x0003
#define AD_ASCII    0x0004
#define AD_ASCIIZ   0x0005
#define AD_PAD      0x0006
#define AD_DEF      0x0007
#define AD_UNDEF    0x0008
#define AD_ENTRY    0x0009
#define AD_ESECT    0x0100

std::unordered_map <std::string, int> directive_id_map = {
    { "org"    , AD_ORG    },
    { "db"     , AD_DB     }, { "byte" , AD_DB },
    { "ds"     , AD_DS     }, { "short", AD_DS },
    { "dl"     , AD_DL     }, { "long" , AD_DL }, { "dw", AD_DL },
    { "ascii"  , AD_ASCII  },
    { "asciiz" , AD_ASCIIZ },
    { "pad"    , AD_PAD    },
    { "def"    , AD_DEF    },
    { "undef"  , AD_UNDEF  },
    { "entry"  , AD_ENTRY  },
    
    // ELF-specific
    { "section", AD_ESECT  }
};

void hv2a_handle_org(hv2a_t* as) {
    int type = 0;

    uint32_t new_pos = hv2a_parse_integer(as, &type);

    if (type == INT_TYPE_SYMBOL) {
        ERROR(0 /* To-do */, "Symbols not allowed for use with .org");
    }

    as->vaddr = new_pos;
    //as->pos = new_pos;

    return;
}

void hv2a_handle_data(hv2a_t* as, int s) {
    int bytes = 1 << (s - 1);

    as->pos += bytes;
    as->vaddr += bytes;

    if (as->pass != 1)
        return;

    while (!as->stream->eof()) {
        uint32_t v = hv2a_parse_integer(as);

        v &= 0xffffffff >> (32 - (bytes * 8));

        as->output->write((char*)&v, bytes);

        while (std::isspace(as->c))
            as->c = as->stream->get();

        if (as->stream->eof()) {
            break;
        } else {
            if (as->c != ',') {
                ERROR(E_EXP_COMMA, "Expected \',\' between operands");
            }
        }

        as->c = as->stream->get();

        hv2a_consume_whitespace(as);
    }
}

#define SHF_NONE                0x0
#define SHF_WRITE               0x1
#define SHF_ALLOC               0x2
#define SHF_EXECINSTR           0x4
#define SHF_MERGE               0x10
#define SHF_STRINGS             0x20
#define SHF_INFO_LINK           0x40
#define SHF_LINK_ORDER          0x80
#define SHF_OS_NONCONFORMING    0x100
#define SHF_GROUP               0x200
#define SHF_TLS                 0x400
#define SHF_MASKOS              0x0ff00000
#define SHF_MASKPROC            0xf0000000
#define SHF_ORDERED             0x4000000
#define SHF_EXCLUDE             0x8000000

#define SHT_NULL                0x0
#define SHT_PROGBITS            0x1
#define SHT_SYMTAB              0x2
#define SHT_STRTAB              0x3
#define SHT_RELA                0x4
#define SHT_HASH                0x5
#define SHT_DYNAMIC             0x6
#define SHT_NOTE                0x7
#define SHT_NOBITS              0x8
#define SHT_REL                 0x9
#define SHT_SHLIB               0x0a
#define SHT_DYNSYM              0x0b
#define SHT_INIT_ARRAY          0x0e
#define SHT_FINI_ARRAY          0x0f
#define SHT_PREINIT_ARRAY       0x10
#define SHT_GROUP               0x11
#define SHT_SYMTAB_SHNDX        0x12
#define SHT_NUM                 0x13
#define SHT_LOOS                0x60000000

std::unordered_map <std::string, uint32_t> section_flags_map = {
    { ".bss",           SHF_ALLOC | SHF_WRITE           },
    { ".comment",       SHF_NONE                        },
    { ".data",          SHF_ALLOC | SHF_WRITE           },
    { ".data1",         SHF_ALLOC | SHF_WRITE           },
    { ".debug",         SHF_NONE                        },
    { ".dynamic",       SHF_ALLOC | SHF_WRITE           },
    { ".dynstr",        SHF_ALLOC                       },
    { ".dynsym",        SHF_ALLOC                       },
    { ".fini",          SHF_ALLOC | SHF_EXECINSTR       },
    { ".fini_array",    SHF_ALLOC | SHF_WRITE           },
    { ".got",           SHF_ALLOC                       },
    { ".hash",          SHF_ALLOC                       },
    { ".init",          SHF_ALLOC | SHF_EXECINSTR       },
    { ".init_array",    SHF_ALLOC | SHF_WRITE           },
    { ".interp",        SHF_ALLOC                       },
    { ".line",          SHF_NONE                        },
    { ".note",          SHF_NONE                        },
    { ".plt",           SHF_ALLOC                       },
    { ".preinit_array", SHF_ALLOC | SHF_WRITE           },
    { ".relname",       SHF_ALLOC                       },
    { ".relaname",      SHF_ALLOC                       },
    { ".rodata",        SHF_ALLOC                       },
    { ".rodata1",       SHF_ALLOC                       },
    { ".shstrtab",      SHF_NONE                        },
    { ".strtab",        SHF_ALLOC                       },
    { ".symtab",        SHF_ALLOC                       },
    { ".symtab_shndx",  SHF_ALLOC                       },
    { ".tbss",          SHF_ALLOC | SHF_WRITE | SHF_TLS },
    { ".tdata",         SHF_ALLOC | SHF_WRITE | SHF_TLS },
    { ".tdata1",        SHF_ALLOC | SHF_WRITE | SHF_TLS },
    { ".text",          SHF_ALLOC | SHF_EXECINSTR       }
};

std::unordered_map <std::string, uint32_t> section_type_map = {
    { ".bss",           SHT_NOBITS        },
    { ".comment",       SHT_PROGBITS      },
    { ".data",          SHT_PROGBITS      },
    { ".data1",         SHT_PROGBITS      },
    { ".debug",         SHT_PROGBITS      },
    { ".dynamic",       SHT_DYNAMIC       },
    { ".dynstr",        SHT_STRTAB        },
    { ".dynsym",        SHT_DYNSYM        },
    { ".fini",          SHT_PROGBITS      },
    { ".fini_array",    SHT_FINI_ARRAY    },
    { ".got",           SHT_PROGBITS      },
    { ".hash",          SHT_HASH          },
    { ".init",          SHT_PROGBITS      },
    { ".init_array",    SHT_INIT_ARRAY    },
    { ".interp",        SHT_PROGBITS      },
    { ".line",          SHT_PROGBITS      },
    { ".note",          SHT_NOTE          },
    { ".plt",           SHT_PROGBITS      },
    { ".preinit_array", SHT_PREINIT_ARRAY },
    { ".relname",       SHT_REL           },
    { ".relaname",      SHT_RELA          },
    { ".rodata",        SHT_PROGBITS      },
    { ".rodata1",       SHT_PROGBITS      },
    { ".shstrtab",      SHT_STRTAB        },
    { ".strtab",        SHT_STRTAB        },
    { ".symtab",        SHT_SYMTAB        },
    { ".symtab_shndx",  SHT_SYMTAB_SHNDX  },
    { ".tbss",          SHT_NOBITS        },
    { ".tdata",         SHT_PROGBITS      },
    { ".tdata1",        SHT_PROGBITS      },
    { ".text",          SHT_PROGBITS      }
};

std::unordered_map <std::string, uint32_t> section_type_name_map = {
    { "null",          0x0        },
    { "progbits",      0x1        },
    { "symtab",        0x2        },
    { "strtab",        0x3        },
    { "rela",          0x4        },
    { "hash",          0x5        },
    { "dynamic",       0x6        },
    { "note",          0x7        },
    { "nobits",        0x8        },
    { "rel",           0x9        },
    { "shlib",         0x0a       },
    { "dynsym",        0x0b       },
    { "init_array",    0x0e       },
    { "fini_array",    0x0f       },
    { "preinit_array", 0x10       },
    { "group",         0x11       },
    { "symtab_shndx",  0x12       },
    { "num",           0x13       },
    { "loos",          0x60000000 }
};

void hv2a_handle_section(hv2a_t* as) {
    if (as->pass != 1)
        return;

    elf_section_t sect;

    as->sections.back().hdr.sh_size = as->pos - as->sections.back().hdr.sh_offset;

    sect.hdr.sh_addr = as->vaddr;
    sect.hdr.sh_offset = as->pos;
    sect.hdr.sh_addralign = 4;
    sect.hdr.sh_addr = as->vaddr;

    while ((!std::isspace(as->c)) && (as->c != ',') && (!as->stream->eof())) {
        sect.name.push_back(as->c);

        as->c = as->stream->get();
    }

    while (std::isspace(as->c))
        as->c = as->stream->get();

    bool standard_section = false;

    if (section_flags_map.contains(sect.name)) {
        sect.hdr.sh_flags = section_flags_map[sect.name];
        sect.hdr.sh_type = section_type_map[sect.name];

        standard_section = true;
    } else {
        sect.hdr.sh_flags = SHF_ALLOC | SHF_WRITE;
    }

    if (as->stream->eof()) {
        as->sections.push_back(sect);

        return;
    } else {
        if (as->c != ',') {
            ERROR(0 /* To-do*/, "Expected \',\'");
        }
    }

    if (standard_section) {
        _hv2_log(warning, "Overriding standard section flags");
    }

    as->c = as->stream->get();

    hv2a_consume_whitespace(as);

    std::string flags = hv2a_parse_string(as);

    for (char c : flags) {
        switch (c) {
            case 'a': sect.hdr.sh_flags |= SHF_ALLOC; break;
            case 'w': sect.hdr.sh_flags |= SHF_WRITE; break;
            case 'x': sect.hdr.sh_flags |= SHF_EXECINSTR; break;
            default: {
                ERROR(0 /* To-do*/, "Invalid ELF section flag \'%c\'", c);
            } break;
        }
    }

    while (std::isspace(as->c))
        as->c = as->stream->get();

    if (as->stream->eof()) {
        as->sections.push_back(sect);

        return;
    } else {
        if (as->c != ',') {
            ERROR(0 /* To-do*/, "Expected \',\'");
        }
    }

    if (standard_section) {
        _hv2_log(warning, "Overriding standard section type");
    }

    as->c = as->stream->get();

    hv2a_consume_whitespace(as);

    if (as->c != '@') {
        ERROR(0 /* To-do*/, "Expected \'@\'");
    }

    std::string type;

    as->c = as->stream->get();

    while (std::isalpha(as->c)) {
        type.push_back(as->c);
        
        as->c = as->stream->get();
    }

    if (!section_type_name_map.contains(type)) {
        ERROR(0 /* To-do */, "Invalid type mask name \"%s\"", type.c_str());
    }

    sect.hdr.sh_type = section_type_name_map[type];

    as->sections.push_back(sect);
}

void hv2a_handle_asciiz(hv2a_t* as, bool zt) {
    if (as->pass != 1)
        return;

    std::string str = hv2a_parse_string(as);

    as->output->write(str.c_str(), str.size());

    as->pos += str.size();
    as->vaddr += str.size();

    char zero = '\0';

    if (zt) {
        as->output->write("\0", sizeof(char));

        as->pos++;
        as->vaddr++;
    }
}

void hv2a_handle_entry(hv2a_t* as) {
    if (as->pass != 1)
        return;

    as->entry = hv2a_parse_integer(as);
}

bool hv2a_handle_directive(hv2a_t* as) {
    as->c = as->stream->get();

    std::string buf;

    if (!(std::isalpha(as->c) || (as->c == '_'))) {
        ERROR(0 /*To-do*/, "Expected directive");
    }

    while (std::isalnum(as->c) || (as->c == '_')) {
        buf.push_back(as->c);

        as->c = as->stream->get();
    }

    // Local label
    if (!directive_id_map.contains(buf)) {
        hv2a_consume_whitespace(as);

        if (as->c != ':') {
            ERROR(0 /*To-do*/, "Expected \':\' after local label");
        }

        if (as->pass == 0)
            as->local_symbols.insert({ buf + as->current_symbol, as->vaddr });
        
        return true;
    }

    int id = directive_id_map[buf];

    hv2a_consume_whitespace(as);

    switch (id) {
        case AD_ORG: {
            hv2a_handle_org(as);
        } break;

        case AD_DB: case AD_DS: case AD_DL: {
            hv2a_handle_data(as, id);
        } break;
        
        case AD_ASCII: case AD_ASCIIZ: {
            hv2a_handle_asciiz(as, id - AD_ASCII);
        } break;

        case AD_ENTRY: {
            hv2a_handle_entry(as);
        } break;
        
        case AD_ESECT: {
            hv2a_handle_section(as);
        } break;
    }

    return false;
}

void hv2a_setup_next_pass(hv2a_t* as) {
    // Set last section size
    as->sections.back().hdr.sh_size = as->pos - as->sections.back().hdr.sh_offset;

    as->pass = (as->pass + 1) % 2;
    as->pos = 0;
    as->vaddr = 0;
    as->current_symbol = "none";
}

operand_data_t hv2a_parse_operands(hv2a_t* as) {
    operand_data_t od;

    while (!as->stream->eof()) {
        uint32_t type = hv2a_get_operand_type(as);

        switch (type) {
            case OPR_INT: {
                uint32_t i = hv2a_parse_integer(as);

                if (od.mode == OPR_NONE) { od.integer[0] = i; od.mode = OPR_INT1; break; }
                if (od.mode == OPR_INT1) { od.integer[1] = i; od.mode = OPR_INT2; break; }
                if (od.mode == OPR_INT2) { od.integer[2] = i; od.mode = OPR_INT3; break; }
                if (od.mode == OPR_IDX1) { od.integer[0] = i; od.mode = OPR_IDX_INT; break; }

                ERROR(0, "Unknown operand mode");
            } break;

            case OPR_IDX: {
                hv2a_parse_idx(as, &od);

                if (od.mode == OPR_NONE) { od.mode = OPR_IDX1; break; }
                if (od.mode == OPR_INT1) { od.mode = OPR_IDX_INT; break; }

                ERROR(0, "Unknown operand mode");
            } break;

            default: {
                ERROR(0, "Unimplemented operand type");
            } break;
        }

        while (std::isspace(as->c))
            as->c = as->stream->get();

        //hv2a_consume_whitespace(as);

        if (as->stream->eof()) {
            break;
        } else {
            if (as->c != ',') {
                ERROR(E_EXP_COMMA, "Expected \',\' between operands");
            }
        }

        as->c = as->stream->get();

        hv2a_consume_whitespace(as);
    }

    return od;
}

#define PUSH(...) { \
    char buf[1024]; \
    std::sprintf(buf, __VA_ARGS__); \
    std::istringstream i(buf); \
    as->stream = &i; \
    hv2a_assemble(as); }

// hv2a_encode_pseudo_op to-do: Check operand format
void hv2a_encode_pseudo_op(hv2a_t* as, mnemonic_data_t* md, operand_data_t* od) {
    switch (md->pseudo_op) {
        case PSD_B: {
            PUSH("beq       r0, r0, 0x%08x", od->integer[0]);
        } break;

        case PSD_CALLR: {
            PUSH("sub.u     sp, 4");
            PUSH("store.l   [sp], pc");
            PUSH("nop       r0");
            PUSH("move      pc, %u", od->integer[0]);
        } break;

        case PSD_CALLI: {
            PUSH("sub.u     sp, 4");
            PUSH("store.l   [sp], pc");
            PUSH("li.w      at, 0x%08x", od->integer[0]);
            PUSH("move      pc, at");
        } break;

        case PSD_CLR: {
            PUSH("xor.u     %u, %u, %u", od->integer[0], od->integer[0], od->integer[0]);
        } break;

        case PSD_DEC: {
            PUSH("sub.u     %u, 1", od->integer[0]);
        } break;
        
        case PSD_INC: {
            PUSH("add.u     %u, 1", od->integer[0]);
        } break;

        case PSD_LIW: {
            PUSH("li.u      %u, 0x%08x", od->integer[0], od->integer[1] & 0xffff0000);
            PUSH("or.u      %u, 0x%08x", od->integer[0], od->integer[1] & 0x0000ffff);
        } break;

        case PSD_MOVE: {
            PUSH("add.u     %u, r0, %u", od->integer[0], od->integer[1]);
        } break;

        case PSD_NOP: {
            PUSH("add.u     r0, r0, r0");
        } break;

        case PSD_NOT: {
            PUSH("xor.s     %u, 0xffff", od->integer[0]);
        } break;

        case PSD_POP: {
            PUSH("add.u     sp, 4");
            PUSH("load.l    %u, [sp-4]", od->integer[0]);
        } break;

        case PSD_PUSH: {
            PUSH("sub.u     sp, 4");
            PUSH("store.l   [sp], %u", od->integer[0]);
        } break;

        case PSD_RET: {
            PUSH("add.u     sp, 4");
            PUSH("load.l    at, [sp-4]");
            PUSH("add.u     at, %u", hv2a_get_pipeline_offset(as));
            PUSH("move      pc, at");
        } break;

        case PSD_SWAP: {
            PUSH("rol.u     %u, 16", od->integer[0]);
        } break;

        case PSD_ZX: {
            if (md->pseudo_data[0] == 0) {
                // zx.b
                PUSH("and.u %u, 0xff", od->integer[0]);
            } else if (md->pseudo_data[0] == 1) {
                // zx.s
                PUSH("and.u %u, 0xffff", od->integer[0]);
            }
        } break;

        case PSD_BI: {
            /* To-do */
        };

        // Use XOR swap
        case PSD_XCH: {
            PUSH("xor.u %u, %u, %u", od->integer[0], od->integer[0], od->integer[1]);
            PUSH("xor.u %u, %u, %u", od->integer[1], od->integer[1], od->integer[0]);
            PUSH("xor.u %u, %u, %u", od->integer[0], od->integer[0], od->integer[1]);
        } break;
    }
}

#undef PUSH

void hv2a_assemble(hv2a_t* as) {
    if (as->stream->eof())
        return;

    as->c = as->stream->get();

    while (std::isspace(as->c))
        as->c = as->stream->get();

    if (as->stream->eof())
        return;
    
    std::string buf;

    // Assembler directive
    if (as->c == '.') {
        if (hv2a_handle_directive(as)) {
            hv2a_assemble(as);
        }
    
        return;
    }

    if (!(std::isalpha(as->c) || (as->c == '_'))) {
        ERROR(E_EXP_MNEMONIC_OR_LABEL, "Expected mnemonic or label");
    }

    while (std::isalnum(as->c) || (as->c == '_') || (as->c == '.')) {
        buf.push_back(as->c);

        as->c = as->stream->get();
    }

    hv2a_consume_whitespace(as);

    // Label
    if (as->c == ':') {
        as->current_symbol = buf;

        if (as->pass == 0)
            as->global_symbols.insert({ buf, as->vaddr });

        hv2a_assemble(as);

        return;
    }

    // Instruction
    mnemonic_data_t md;
    operand_data_t od;

    if (mnemonic_data_map.contains(buf)) {
        // Encode normal instruction
        if (as->pass == 1) {
            md = mnemonic_data_map[buf];

            md.mnemonic = buf;

            od = hv2a_parse_operands(as);

            uint32_t opcode = encode_instruction(&md, &od);

            as->output->write((char*)&opcode, sizeof(uint32_t));
        }

        as->vaddr += 4;
        as->pos += 4;
    } else if (pseudo_data_map.contains(buf)) {
        md = pseudo_data_map[buf];

        md.mnemonic = buf;

        od = hv2a_parse_operands(as);

        hv2a_encode_pseudo_op(as, &md, &od);
    } else {
        ERROR(E_UNKNOWN_MNEMONIC, "Unknown instruction \"%s\"", buf.c_str());
    }
}

#define HV2A_OUTPUT_FORMAT_RAW      0
#define HV2A_OUTPUT_FORMAT_ELF32    1
#define HV2A_OUTPUT_FORMAT_COFF     2 /* Unimplemented */

void hv2a_set_output_format(hv2a_t* as, uint32_t fmt) {
    as->output_format = fmt;
}

uint32_t hv2a_get_shstrtab_size(hv2a_t* as) {
    uint32_t size = 0;

    for (elf_section_t& s : as->sections) {
        size += s.name.size() + 1; // Account for zero byte
    }

    return size;
}

#define STACK_BASE 0xc0000000
#define STACK_SIZE 0x80000 // 512 KiB
#define PHDR_COUNT 3
#define ELF_HDR_SIZE (sizeof(elf32_hdr_t) + (sizeof(elf32_phdr_t) * PHDR_COUNT))

uint32_t hv2a_assemble_stream_impl(hv2a_t* as, std::istream* input, std::iostream* output) {
    uint32_t pos = 0;

    as->output = output;

    // Assemble program
    for (int p = 0; p < 2; p++) {
        //_hv2_log(debug, "Pass %u", p);

        while (!input->eof()) {
            std::string line;
            std::istringstream stream;

            std::getline(*input, line);

            stream.str(line);

            as->stream = &stream;

            //_hv2_log(info, "Assembling %s", stream.str().c_str());

            hv2a_assemble(as);
        }

        pos = as->pos;

        hv2a_setup_next_pass(as);

        input->clear();
        input->seekg(0);
    }

    return pos;
}

void hv2a_assemble_raw(hv2a_t* as, std::istream* input, std::ostream* output) {
    std::stringstream buf;

    // Assemble input to buffer
    hv2a_assemble_stream_impl(as, input, &buf);

    // Write text section
    as->output->clear();
    as->output->seekg(0);

    char c = as->output->get();

    while (!as->output->eof()) {        
        output->write(&c, sizeof(char));

        c = as->output->get();
    }
}

void hv2a_assemble_elf32(hv2a_t* as, std::istream* input, std::ostream* output) {
    elf32_hdr_t hdr;

    // 0 = text
    // 1 = rodata
    // 2 = stack
    elf32_phdr_t phdr[PHDR_COUNT];

    std::stringstream buf;

    // Assemble input to buffer
    uint32_t pos = hv2a_assemble_stream_impl(as, input, &buf);
    
    // Prepare ELF data
    elf_section_t sect[2];

    // Fix section offsets and find .text and .rodata
    for (elf_section_t& s : as->sections) {
        if (s.name.size()) {
            s.hdr.sh_offset += ELF_HDR_SIZE;
        }

        if (s.name == ".text") sect[0] = s;
        if (s.name == ".rodata") sect[1] = s;
    }

    // Create shstrtab section
    elf_section_t strtab;

    strtab.name             = ".shstrtab";
    strtab.hdr.sh_name      = 0;
    strtab.hdr.sh_type      = SHT_STRTAB;
    strtab.hdr.sh_flags     = SHF_STRINGS;
    strtab.hdr.sh_offset    = pos + ELF_HDR_SIZE;
    strtab.hdr.sh_addr      = 0x00000000; // Section not mapped
    strtab.hdr.sh_addralign = 1;

    // Calculate shstrtab size taking into account
    // it's own name
    uint32_t shstrtab_size = hv2a_get_shstrtab_size(as) + strtab.name.size() + 1;
    strtab.hdr.sh_size      = shstrtab_size;

    // Push shstrtab
    as->sections.push_back(strtab);

    // List sections (remove)
    // for (elf_section_t& s : as->sections) {
    //     _hv2_log(debug, "section %s start=%08x, vaddr=%08x, size=%08x", s.name.c_str(), s.hdr.sh_offset, s.vaddr, s.hdr.sh_size);
    // }

    // Fill in header info
    hdr.e_type          = ET_EXEC;
    hdr.e_entry         = as->entry;
    hdr.e_phnum         = PHDR_COUNT;
    hdr.e_phentsize     = sizeof(elf32_phdr_t);

    // .text segment
    phdr[0].p_align     = 32;
    phdr[0].p_filesz    = sect[0].hdr.sh_size;
    phdr[0].p_memsz     = sect[0].hdr.sh_size;
    phdr[0].p_offset    = sect[0].hdr.sh_offset;
    phdr[0].p_paddr     = sect[0].hdr.sh_offset;
    phdr[0].p_vaddr     = sect[0].hdr.sh_addr;
    phdr[0].p_type      = PT_LOAD;
    phdr[0].p_flags     = PF_X | PF_R | PF_W;

    // .rodata segment
    phdr[1].p_align     = 1;
    phdr[1].p_filesz    = sect[1].hdr.sh_size;
    phdr[1].p_memsz     = sect[1].hdr.sh_size;
    phdr[1].p_offset    = sect[1].hdr.sh_offset;
    phdr[1].p_paddr     = sect[1].hdr.sh_offset;
    phdr[1].p_vaddr     = sect[1].hdr.sh_addr;
    phdr[1].p_type      = PT_LOAD;
    phdr[1].p_flags     = PF_R;

    // Stack segment
    phdr[2].p_align     = 32;
    phdr[2].p_filesz    = 1;
    phdr[2].p_memsz     = STACK_SIZE;
    phdr[2].p_offset    = 7;
    phdr[2].p_paddr     = 0;
    phdr[2].p_vaddr     = STACK_BASE - STACK_SIZE;
    phdr[2].p_type      = PT_LOAD;
    phdr[2].p_flags     = PF_R | PF_W;

    // Fill in section header info
    hdr.e_shentsize = sizeof(elf32_shdr_t);
    hdr.e_shnum     = as->sections.size();
    hdr.e_shstrndx  = as->sections.size() - 1;
    hdr.e_shoff     = pos + ELF_HDR_SIZE + shstrtab_size;

    // Start writing stuff, write ELF and program headers
    output->write((char*)&hdr, sizeof(elf32_hdr_t));
    output->write((char*)&phdr, sizeof(elf32_phdr_t) * PHDR_COUNT);

    // Write text section
    as->output->clear();
    as->output->seekg(0);

    char c = as->output->get();

    while (!as->output->eof()) {        
        output->write(&c, sizeof(char));

        c = as->output->get();
    }

    // This keeps track of offsets within shstrtab
    int shstrtab_off = 0;
    
    // Write shstrtab section
    for (elf_section_t& s : as->sections) {
        s.shstrtab_off = shstrtab_off;

        if (s.name.size()) {
            output->write(s.name.c_str(), s.name.size());
        }

        output->write("\0", sizeof(char));

        shstrtab_off += 1 + s.name.size();
    }

    // Write section headers
    for (int idx = 0; idx < as->sections.size(); idx++) {
        elf_section_t& s = as->sections[idx];

        s.hdr.sh_name = s.shstrtab_off;

        output->write((char*)&s.hdr, sizeof(elf32_shdr_t));
    }
}

// This file is part of the hs compiler
#include "../assembler.hpp"

namespace hs {
    class assembler_hv2_t : public assembler_t {
        std::istream* m_input;
        std::ostream* m_output;
        error_logger_t* m_logger;
        cli_parser_t* m_cli;
        hv2a_t* m_assembler;

        int pipeline_size = 3;
        bool pipeline_flush = false;

    public:
        ~assembler_hv2_t() {
            delete(m_assembler);
        }

        void init(std::istream* input, std::ostream* output, error_logger_t* logger, cli_parser_t* cli) override {
            _hv2_log::init("hv2_assembler");

            m_input = input;
            m_output = output;
            m_logger = logger;
            m_cli = cli;
            m_assembler = new hv2a_t;

            std::string opt = m_cli->get_setting(ST_XASM);

            // Parse options
            if (opt.size()) {
                std::stringstream stream(opt);

                std::vector <std::string> options;
                std::string buf;

                char c = stream.get();

                while (!stream.eof()) {
                    if (c == ',') {
                        if (buf.size()) {
                            options.push_back(buf);

                            buf.clear();
                        }

                        c = stream.get();
                    }

                    buf.push_back(c);

                    c = stream.get();
                }

                if (buf.size()) {
                    options.push_back(buf);
                }

                for (std::string& o : options) {
                    switch (o[1]) {
                        case 'P': {
                            pipeline_size = std::stoi(&o[2]);
                        } break;

                        case 'f': case 'F': {
                            pipeline_flush = o[1] == 'F';
                        } break;

                        default: {
                            _hv2_log(warning, "Unknown assembler setting \'%c\'", o[1]);
                        };
                    }
                }
            }

            m_assembler->pipeline_size = pipeline_size;
            m_assembler->flush = pipeline_flush;
        }

        void assemble() override {
            if (m_cli->get_setting(ST_OUTPUT_FORMAT) == "elf32") {
                hv2a_assemble_elf32(m_assembler, m_input, m_output);
            } else {
                hv2a_assemble_raw(m_assembler, m_input, m_output);
            }
        };
    };
}