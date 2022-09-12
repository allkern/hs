#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>


namespace hs {
    enum hyrisc_operand_mode_t : int {
        OP_RX,      // r0
        OP_RXRY,    // r0, r1
        OP_RXI16,   // r0, 0x0000
        OP_RXRYRZ,  // r0, r1, r2
        OP_RXRYI8,  // r0, r1, 0x00
        OP_RXIND,   // r0, [r1+r2*i5] / [r0+r1*i5], r2
        OP_RXFIX,   // r0, [r1-i10] / [r0-i10], r1
        OP_I16,     // 0x0000
        OP_INDEX,   // [r0+r1*i5]
        OP_RANGE,   // {r0-r1}
        OP_NONE     //
    };

    enum hyrisc_encoding_t : int {
        ENC_4,
        ENC_3,
        ENC_2,
        ENC_1
    };

    hyrisc_encoding_t hyrisc_mode_encoding[14] = {
        ENC_4,    // OP_RX
        ENC_4,    // OP_RXRY
        ENC_2,    // OP_RXI16
        ENC_4,    // OP_RXRYRZ
        ENC_3,    // OP_RXRYI8
        ENC_4,    // OP_RXIND
        ENC_4,    // OP_RXFIXA
        ENC_4,    // OP_RXFIXS
        ENC_2,    // OP_I16
        ENC_2,    // OP_CONDI
        ENC_4,    // OP_CONDR
        ENC_4,    // OP_COND
        ENC_4,    // OP_RANGE
        ENC_4     // OP_NONE
    };

    enum hyrisc_opcode_t {
        HY_MOV       = 0xff,
        HY_LI        = 0xfe,
        HY_LUI       = 0xfd,
        HY_LOADM     = 0xfc, // LOAD Multiply
        HY_LOADS     = 0xfb, // LOAD Shift
        HY_LOADFA    = 0xfa, // LOAD Fixed Add
        HY_LOADFS    = 0xf9, // LOAD Fixed Sub
        HY_STOREM    = 0xf8, // STORE Multiply
        HY_STORES    = 0xf7, // STORE Shift
        HY_STOREFA   = 0xf6, // STORE Fixed Add
        HY_STOREFS   = 0xf5, // STORE Fixed Sub
        HY_LEAM      = 0xf4, // LEA Multiply
        HY_LEAS      = 0xf3, // LEA Shift
        HY_ADDR      = 0xef, // ADD Register
        HY_ADDUI8    = 0xee, // ADD Unsigned Immediate 8
        HY_ADDUI16   = 0xed, // ADD Unsigned Immediate 16
        HY_ADDSI8    = 0xec, // ADD Signed Immediate 8
        HY_ADDSI16   = 0xeb, // ADD Signed Immediate 16
        HY_SUBR      = 0xea,
        HY_SUBUI8    = 0xe9,
        HY_SUBUI16   = 0xe8,
        HY_SUBSI8    = 0xe7,
        HY_SUBSI16   = 0xe6,
        HY_MULR      = 0xe5,
        HY_MULUI8    = 0xe4,
        HY_MULUI16   = 0xe3,
        HY_MULSI8    = 0xe2,
        HY_MULSI16   = 0xe1,
        HY_DIVR      = 0xe0,
        HY_DIVUI8    = 0xdf,
        HY_DIVUI16   = 0xde,
        HY_DIVSI8    = 0xdd,
        HY_DIVSI16   = 0xdc,
        HY_CMPZ      = 0xdb, // Compare Zero
        HY_CMPR      = 0xda,
        HY_CMPI8     = 0xd9,
        HY_CMPI16    = 0xd8,
        HY_ANDR      = 0xcf,
        HY_ANDI8     = 0xce,
        HY_ANDI16    = 0xcd,
        HY_ORR       = 0xcc,
        HY_ORI8      = 0xcb,
        HY_ORI16     = 0xca,
        HY_XORR      = 0xc9,
        HY_XORI8     = 0xc8,
        HY_XORI16    = 0xc7,
        HY_NOT       = 0xc6,
        HY_NEG       = 0xc5,
        HY_SEXT      = 0xc4,
        HY_ZEXT      = 0xc3,
        HY_RSTS      = 0xc2, // RST Single
        HY_RSTM      = 0xc1, // RST Multiple
        HY_INC       = 0xc0,
        HY_DEC       = 0xbf,
        HY_TST       = 0xbe,
        HY_LSLR      = 0xbd,
        HY_LSLI16    = 0xbc,
        HY_LSRR      = 0xbb,
        HY_LSRI16    = 0xba,
        HY_ASLR      = 0xb9,
        HY_ASLI16    = 0xb8,
        HY_ASRR      = 0xb7,
        HY_ASRI16    = 0xb6,
        HY_BCCS      = 0xaf, // BCC Signed 
        HY_BCCU      = 0xae, // BCC Unsigned
        HY_JALCCI16  = 0xad, // JAL Immediate 16
        HY_JALCCM    = 0xac, // JAL Multiply
        HY_JALCCS    = 0xab, // JAL Shift
        HY_CALLCCI16 = 0xaa,
        HY_CALLCCM   = 0xa9,
        HY_CALLCCS   = 0xa8,
        HY_RTLCC     = 0xa7,
        HY_RETCC     = 0xa6,
        HY_PUSHM     = 0x9f,
        HY_POPM      = 0x9e,
        HY_PUSHS     = 0x9d,
        HY_POPS      = 0x9c,
        HY_NOP       = 0x8f,
        HY_DEBUG     = 0x45,
        HY_BAD       = 0x00
    };

    enum hyrisc_mnemonic_t {
        IM_MOV,
        IM_LI,
        IM_LUI,
        IM_LOAD,
        IM_STORE,
        IM_LEA,
        IM_ADD,
        IM_SUB,
        IM_MUL,
        IM_DIV,
        IM_CMPZ,
        IM_CMP,
        IM_AND,
        IM_OR,
        IM_XOR,
        IM_NOT,
        IM_NEG,
        IM_SEXT,
        IM_ZEXT, // Removed
        IM_RST,
        IM_INC,
        IM_DEC,
        IM_TST,
        IM_LSL,
        IM_LSR,
        IM_ASL,
        IM_ASR,
        //IM_BEQ,
        //IM_BNE,
        //IM_BCS,
        //IM_BCC,
        //IM_BMI,
        //IM_BPL,
        //IM_BVS,
        //IM_BVC,
        //IM_BHI,
        //IM_BLS,
        //IM_BGE,
        //IM_BLT,
        //IM_BGT,
        //IM_BLE,
        IM_BCC,
        IM_BRA,
        //IM_JALEQ,
        //IM_JALNE,
        //IM_JALCS,
        //IM_JALCC,
        //IM_JALMI,
        //IM_JALPL,
        //IM_JALVS,
        //IM_JALVC,
        //IM_JALHI,
        //IM_JALLS,
        //IM_JALGE,
        //IM_JALLT,
        //IM_JALGT,
        //IM_JALLE,
        IM_JALCC,
        IM_JAL,
        //IM_CALLEQ,
        //IM_CALLNE,
        //IM_CALLCS,
        //IM_CALLCC,
        //IM_CALLMI,
        //IM_CALLPL,
        //IM_CALLVS,
        //IM_CALLVC,
        //IM_CALLHI,
        //IM_CALLLS,
        //IM_CALLGE,
        //IM_CALLLT,
        //IM_CALLGT,
        //IM_CALLLE,
        IM_CALLCC,
        IM_CALL,
        //IM_RTLEQ,
        //IM_RTLNE,
        //IM_RTLCS,
        //IM_RTLCC,
        //IM_RTLMI,
        //IM_RTLPL,
        //IM_RTLVS,
        //IM_RTLVC,
        //IM_RTLHI,
        //IM_RTLLS,
        //IM_RTLGE,
        //IM_RTLLT,
        //IM_RTLGT,
        //IM_RTLLE,
        IM_RTLCC,
        IM_RTL,
        //IM_RETEQ,
        //IM_RETNE,
        //IM_RETCS,
        //IM_RETCC,
        //IM_RETMI,
        //IM_RETPL,
        //IM_RETVS,
        //IM_RETVC,
        //IM_RETHI,
        //IM_RETLS,
        //IM_RETGE,
        //IM_RETLT,
        //IM_RETGT,
        //IM_RETLE,
        IM_RETCC,
        IM_RET,
        IM_PUSH,
        IM_POP,
        IM_NOP,      
        IM_BAD,      
        IM_DEBUG      
    };

    std::unordered_map <std::string, hyrisc_mnemonic_t> hyrisc_mnemonic_id = {
        { "mov"   , IM_MOV },
        { "li"    , IM_LI },
        { "lui"   , IM_LUI },
        { "load"  , IM_LOAD },
        { "store" , IM_STORE },
        { "lea"   , IM_LEA },
        { "add"   , IM_ADD },
        { "sub"   , IM_SUB },
        { "mul"   , IM_MUL },
        { "div"   , IM_DIV },
        { "cmpz"  , IM_CMPZ },
        { "cmp"   , IM_CMP },
        { "and"   , IM_AND },
        { "or"    , IM_OR },
        { "xor"   , IM_XOR },
        { "not"   , IM_NOT },
        { "neg"   , IM_NEG },
        { "sext"  , IM_SEXT },
        { "zext"  , IM_ZEXT },
        { "rst"   , IM_RST },
        { "inc"   , IM_INC },
        { "dec"   , IM_DEC },
        { "tst"   , IM_TST },
        { "lsl"   , IM_LSL },
        { "lsr"   , IM_LSR },
        { "asl"   , IM_ASL },
        { "asr"   , IM_ASR },
        { "beq"   , IM_BCC },
        { "bne"   , IM_BCC },
        { "bcs"   , IM_BCC },
        { "bcc"   , IM_BCC },
        { "bmi"   , IM_BCC },
        { "bpl"   , IM_BCC },
        { "bvs"   , IM_BCC },
        { "bvc"   , IM_BCC },
        { "bhi"   , IM_BCC },
        { "bls"   , IM_BCC },
        { "bge"   , IM_BCC },
        { "blt"   , IM_BCC },
        { "bgt"   , IM_BCC },
        { "ble"   , IM_BCC },
        { "bra"   , IM_BRA },
        { "jaleq" , IM_JALCC },
        { "jalne" , IM_JALCC },
        { "jalcs" , IM_JALCC },
        { "jalcc" , IM_JALCC },
        { "jalmi" , IM_JALCC },
        { "jalpl" , IM_JALCC },
        { "jalvs" , IM_JALCC },
        { "jalvc" , IM_JALCC },
        { "jalhi" , IM_JALCC },
        { "jalls" , IM_JALCC },
        { "jalge" , IM_JALCC },
        { "jallt" , IM_JALCC },
        { "jalgt" , IM_JALCC },
        { "jalle" , IM_JALCC },
        { "jalra" , IM_JAL },
        { "calleq", IM_CALLCC },
        { "callne", IM_CALLCC },
        { "callcs", IM_CALLCC },
        { "callcc", IM_CALLCC },
        { "callmi", IM_CALLCC },
        { "callpl", IM_CALLCC },
        { "callvs", IM_CALLCC },
        { "callvc", IM_CALLCC },
        { "callhi", IM_CALLCC },
        { "callls", IM_CALLCC },
        { "callge", IM_CALLCC },
        { "calllt", IM_CALLCC },
        { "callgt", IM_CALLCC },
        { "callle", IM_CALLCC },
        { "call"  , IM_CALL },
        { "rtleq" , IM_RTLCC },
        { "rtlne" , IM_RTLCC },
        { "rtlcs" , IM_RTLCC },
        { "rtlcc" , IM_RTLCC },
        { "rtlmi" , IM_RTLCC },
        { "rtlpl" , IM_RTLCC },
        { "rtlvs" , IM_RTLCC },
        { "rtlvc" , IM_RTLCC },
        { "rtlhi" , IM_RTLCC },
        { "rtlls" , IM_RTLCC },
        { "rtlge" , IM_RTLCC },
        { "rtllt" , IM_RTLCC },
        { "rtlgt" , IM_RTLCC },
        { "rtlle" , IM_RTLCC },
        { "rtl"   , IM_RTL },
        { "reteq" , IM_RETCC },
        { "retne" , IM_RETCC },
        { "retcs" , IM_RETCC },
        { "retcc" , IM_RETCC },
        { "retmi" , IM_RETCC },
        { "retpl" , IM_RETCC },
        { "retvs" , IM_RETCC },
        { "retvc" , IM_RETCC },
        { "rethi" , IM_RETCC },
        { "retls" , IM_RETCC },
        { "retge" , IM_RETCC },
        { "retlt" , IM_RETCC },
        { "retgt" , IM_RETCC },
        { "retle" , IM_RETCC },
        { "ret"   , IM_RET },
        { "push"  , IM_PUSH },
        { "pop"   , IM_POP },
        { "nop"   , IM_NOP },
        { "debug" , IM_DEBUG },
        { "bad"   , IM_BAD }
    };
}