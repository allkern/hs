#pragma once

#include <unordered_map>
#include <iostream>
#include <string>

#include "../../error.hpp"
#include "instructions.hpp"

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "assembler", \
        msg, \
        0, 0, 0, false, true \
    );

namespace hs {
    static std::string hyrisc_mode_str[] = {
        "OP_RX",
        "OP_RXRY",
        "OP_RXI16",
        "OP_RXRYRZ",
        "OP_RXRYI8",
        "OP_RXIND",
        "OP_RXFIX",
        "OP_I16",
        "OP_INDEX",
        "OP_RANGE",
        "OP_NONE"
    };
    enum output_format_t {
        F_ELF32,
        F_RAW
    };

    class hyrisc_assembler_t {
        std::istream* m_input;
        std::ostream* m_output;
        char m_current;
        error_logger_t* m_logger;
        std::unordered_map <std::string, uint32_t> m_symbol_map;
        std::unordered_map <std::string, uint32_t> m_local_map;

        int m_pass = 0;
    
        uint32_t m_pos = 0;

        struct instruction_t {
            std::string mnemonic;
            hyrisc_operand_mode_t mode;
            char specifier;
            bool shift = false;
            bool add = false;

            // Data actually on opcode
            uint32_t encoding;
            uint32_t opcode;
            uint32_t fieldx, fieldy, fieldz, fieldw;
            uint32_t imm8;
            uint32_t imm16;
            uint32_t size;
        } m_instruction;

        std::unordered_map <std::string, int> m_cond_map = {
            { "eq", 0  },
            { "ne", 1  },
            { "cs", 2  },
            { "cc", 3  },
            { "mi", 4  },
            { "pl", 5  },
            { "vs", 6  },
            { "vc", 7  },
            { "hi", 8  },
            { "ls", 9  },
            { "ge", 10 },
            { "lt", 11 },
            { "gt", 12 },
            { "le", 13 },
        };

        void parse_opcode() {
            if (!hyrisc_mnemonic_id.contains(m_instruction.mnemonic)) {
                ERROR(fmt("Mnemonic %s not found", m_instruction.mnemonic.c_str()));

                return;

                // Error mnemonic not found
            }

            hyrisc_mnemonic_t id = hyrisc_mnemonic_id[m_instruction.mnemonic];

            switch (id) {
                case IM_ADD: {
                    if ((m_instruction.specifier == 'u') || (m_instruction.specifier == ' ')) {
                        // add.u r0, r1
                        if (m_instruction.mode == OP_RXRY) {
                            m_instruction.opcode = HY_ADDR;
                            m_instruction.fieldz = m_instruction.fieldy;
                            m_instruction.fieldy = m_instruction.fieldx;

                            return;
                        }

                        // add.u r0, r1, r2
                        if (m_instruction.mode == OP_RXRYRZ) {
                            m_instruction.opcode = HY_ADDR;

                            return;
                        }

                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_ADDUI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_ADDUI8;

                            return;
                        }

                        ERROR(fmt("Unsupported ADD.U mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    if (m_instruction.specifier == 's') {
                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_ADDSI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_ADDSI8;

                            return;
                        }

                        ERROR(fmt("Unsupported ADD.S mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for ADD", m_instruction.specifier));
                } break;

                case IM_AND: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_ANDR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_ANDR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_ANDI16;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYI8) {
                        m_instruction.opcode = HY_ANDI8;

                        return;
                    }

                    ERROR(fmt("Unsupported AND mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_ASL: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_ASLR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_ASLR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_ASLI16;

                        return;
                    }

                    ERROR(fmt("Unsupported ASL mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_ASR: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_ASRR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_ASRR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_ASRI16;

                        return;
                    }

                    ERROR(fmt("Unsupported ASR mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_BAD: {
                    m_instruction.opcode = 0x0;

                    return;
                } break;

                case IM_BCC: {
                    std::string cond = m_instruction.mnemonic.substr(m_instruction.mnemonic.size() - 2);
                    std::string uc = m_instruction.mnemonic;

                    for (char& c : uc) c = std::toupper(c);

                    m_instruction.fieldx = m_cond_map[cond];

                    if (m_instruction.specifier == 'u') {
                        if (m_instruction.mode == OP_I16) {
                            m_instruction.opcode = HY_BCCU;

                            return;
                        }

                        ERROR(fmt("Unsupported %s.U mode %s", uc.c_str(), hyrisc_mode_str[m_instruction.mode].c_str()));
                    }
                    
                    if (m_instruction.specifier == 's' || (m_instruction.specifier == ' ')) {
                        if (m_instruction.mode == OP_I16) {
                            m_instruction.opcode = HY_BCCS;

                            return;
                        }

                        ERROR(fmt("Unsupported %s.S mode %s", uc.c_str(), hyrisc_mode_str[m_instruction.mode].c_str()));
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for %s", m_instruction.specifier, uc.c_str()));

                    return;
                } break;

                case IM_BRA: {
                    m_instruction.fieldx = 14;

                    if (m_instruction.specifier == 'u') {
                        if (m_instruction.mode == OP_I16) {
                            m_instruction.opcode = HY_BCCU;

                            return;
                        }

                        ERROR(fmt("Unsupported BRA.U mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));
                    }
                    
                    if ((m_instruction.specifier == 's') || (m_instruction.specifier == ' ')) {
                        if (m_instruction.mode == OP_I16) {
                            m_instruction.opcode = HY_BCCS;

                            return;
                        }

                        ERROR(fmt("Unsupported BRA.S mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for BRA", m_instruction.specifier));

                    return;
                } break;
                
                case IM_CALL: {
                    if (m_instruction.mode == OP_I16) {
                        m_instruction.opcode = HY_CALLCCI16;
                        m_instruction.fieldx = 14;

                        return;
                    }

                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_CALLCCM;
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.fieldx = 14;
                        
                        return;
                    }

                    if (m_instruction.mode == OP_INDEX) {
                        m_instruction.opcode = m_instruction.shift ? HY_CALLCCS : HY_CALLCCM;
                        m_instruction.fieldx = 14;

                        return;
                    }

                    ERROR(fmt("Unsupported CALL mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));
                    
                    return;
                } break;

                case IM_CALLCC: {
                    std::string cond = m_instruction.mnemonic.substr(m_instruction.mnemonic.size() - 2);
                    std::string uc = m_instruction.mnemonic;

                    for (char& c : uc) c = std::toupper(c);

                    if (m_instruction.mode == OP_I16) {
                        m_instruction.opcode = HY_CALLCCI16;
                        m_instruction.fieldx = m_cond_map[cond];

                        return;
                    }

                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_CALLCCM;
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.fieldx = m_cond_map[cond];
                        
                        return;
                    }

                    if (m_instruction.mode == OP_INDEX) {
                        m_instruction.opcode = m_instruction.shift ? HY_CALLCCS : HY_CALLCCM;
                        m_instruction.fieldx = m_cond_map[cond];

                        return;
                    }

                    ERROR(fmt("Unsupported %u mode %s", uc.c_str(), hyrisc_mode_str[m_instruction.mode].c_str()));
                    
                    return;
                } break;

                case IM_CMP: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_CMPR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_CMPI16;

                        return;
                    }

                    ERROR(fmt("Unsupported CMP mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;
                
                case IM_CMPZ: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_CMPZ;

                        return;
                    }

                    ERROR(fmt("Unsupported CMPZ mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_DEBUG: {
                    m_instruction.opcode = HY_DEBUG;

                    return;
                } break;

                case IM_DEC: {
                    m_instruction.opcode = HY_DEC;

                    if (m_instruction.specifier == ' ') { m_instruction.size = 0; }
                    else if (m_instruction.specifier == 'b') { m_instruction.size = 0; }
                    else if (m_instruction.specifier == 's') { m_instruction.size = 1; }
                    else if (m_instruction.specifier == 'l') { m_instruction.size = 2; }
                    else if (m_instruction.specifier == 'd') { m_instruction.size = 3; }
                    else {
                        ERROR(fmt("Unknown specifier \'%c\' for DEC", m_instruction.specifier));

                        return;
                    }

                    return;
                } break;

                case IM_DIV: {
                    if ((m_instruction.specifier == 'u') || (m_instruction.specifier == ' ')) {
                        // add.u r0, r1
                        if (m_instruction.mode == OP_RXRY) {
                            m_instruction.opcode = HY_DIVR;
                            m_instruction.fieldz = m_instruction.fieldy;
                            m_instruction.fieldy = m_instruction.fieldx;

                            return;
                        }

                        // add.u r0, r1, r2
                        if (m_instruction.mode == OP_RXRYRZ) {
                            m_instruction.opcode = HY_DIVR;

                            return;
                        }

                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_DIVUI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_DIVUI8;

                            return;
                        }

                        ERROR(fmt("Unsupported DIV.U mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    if (m_instruction.specifier == 's') {
                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_DIVSI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_DIVSI8;

                            return;
                        }

                        ERROR(fmt("Unsupported DIV.S mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for DIV", m_instruction.specifier));

                    return;
                } break;

                case IM_INC: {
                    m_instruction.opcode = HY_INC;

                    if (m_instruction.specifier == ' ') { m_instruction.size = 0; }
                    else if (m_instruction.specifier == 'b') { m_instruction.size = 0; }
                    else if (m_instruction.specifier == 's') { m_instruction.size = 1; }
                    else if (m_instruction.specifier == 'l') { m_instruction.size = 2; }
                    else if (m_instruction.specifier == 'd') { m_instruction.size = 3; }
                    else {
                        ERROR(fmt("Unknown specifier \'%c\' for INC", m_instruction.specifier));

                        return;
                    }

                    return;
                } break;

                case IM_JAL: {
                    if (m_instruction.mode == OP_I16) {
                        m_instruction.opcode = HY_JALCCI16;
                        m_instruction.fieldx = 14;

                        return;
                    }

                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_JALCCM;
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.fieldx = 14;
                        
                        return;
                    }

                    if (m_instruction.mode == OP_INDEX) {
                        m_instruction.opcode = m_instruction.shift ? HY_JALCCS : HY_JALCCM;
                        m_instruction.fieldx = 14;

                        return;
                    }

                    ERROR(fmt("Unsupported JAL mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));
                    
                    return;
                } break;

                case IM_JALCC: {
                    std::string cond = m_instruction.mnemonic.substr(m_instruction.mnemonic.size() - 2);
                    std::string uc = m_instruction.mnemonic;

                    for (char& c : uc) c = std::toupper(c);

                    if (m_instruction.mode == OP_I16) {
                        m_instruction.opcode = HY_JALCCI16;
                        m_instruction.fieldx = m_cond_map[cond];

                        return;
                    }

                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_JALCCM;
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.fieldx = m_cond_map[cond];
                        
                        return;
                    }

                    if (m_instruction.mode == OP_INDEX) {
                        m_instruction.opcode = m_instruction.shift ? HY_JALCCS : HY_JALCCM;
                        m_instruction.fieldx = m_cond_map[cond];

                        return;
                    }

                    ERROR(fmt("Unsupported %s mode %s", uc.c_str(), hyrisc_mode_str[m_instruction.mode].c_str()));
                    
                    return;
                } break;

                case IM_LEA: {
                    if (m_instruction.mode == OP_RXIND) {
                        m_instruction.opcode = m_instruction.shift ? HY_LEAS : HY_LEAM;

                        return;
                    }

                    if (m_instruction.mode == OP_RXFIX) {
                        m_instruction.opcode = m_instruction.add ? HY_LEAFA : HY_LEAFS;

                        return;
                    }

                    ERROR(fmt("Unsupported LEA mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));
                
                    return;
                } break;

                case IM_LI: {
                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_LI;

                        return;
                    }

                    ERROR(fmt("Unsupported LI mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_LOAD: {
                    if (m_instruction.specifier == ' ') { m_instruction.size = 2; }
                    else if (m_instruction.specifier == 'b') { m_instruction.size = 0; }
                    else if (m_instruction.specifier == 's') { m_instruction.size = 1; }
                    else if (m_instruction.specifier == 'l') { m_instruction.size = 2; }
                    else if (m_instruction.specifier == 'x') { m_instruction.size = 3; }
                    else {
                        ERROR(fmt("Unknown specifier \'%c\' for LOAD", m_instruction.specifier));

                        return;
                    }

                    if (m_instruction.mode == OP_RXIND) {
                        m_instruction.opcode = m_instruction.shift ? HY_LOADS : HY_LOADM;

                        return;
                    }

                    if (m_instruction.mode == OP_RXFIX) {
                        m_instruction.opcode = m_instruction.add ? HY_LOADFA : HY_LOADFS;

                        return;
                    }


                    ERROR(fmt("Unsupported LOAD.%c mode %s", std::toupper(m_instruction.specifier), hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_LSL: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_LSLR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_LSLR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_LSLI16;

                        return;
                    }

                    ERROR(fmt("Unsupported LSL mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_LSR: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_LSRR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_LSRR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_LSRI16;

                        return;
                    }

                    ERROR(fmt("Unsupported LSR mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_LUI: {
                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_LUI;

                        return;
                    }

                    ERROR(fmt("Unsupported LUI mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_MOV: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_MOV;

                        return;
                    }

                    ERROR(fmt("Unsupported MOV mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_MUL: {
                    if ((m_instruction.specifier == 'u') || (m_instruction.specifier == ' ')) {
                        // add.u r0, r1
                        if (m_instruction.mode == OP_RXRY) {
                            m_instruction.opcode = HY_MULR;
                            m_instruction.fieldz = m_instruction.fieldy;
                            m_instruction.fieldy = m_instruction.fieldx;

                            return;
                        }

                        // add.u r0, r1, r2
                        if (m_instruction.mode == OP_RXRYRZ) {
                            m_instruction.opcode = HY_MULR;

                            return;
                        }

                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_MULUI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_MULUI8;

                            return;
                        }

                        ERROR(fmt("Unsupported MUL.U mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    if (m_instruction.specifier == 's') {
                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_MULSI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_MULSI8;

                            return;
                        }

                        ERROR(fmt("Unsupported MUL.S mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for MUL", m_instruction.specifier));

                    return;
                } break;

                case IM_NEG: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.opcode = HY_NEG;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_NEG;

                        return;
                    }

                   ERROR(fmt("Unsupported NEG mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_NOP: {
                    if (m_instruction.mode == OP_NONE) {
                        m_instruction.opcode = HY_NOP;

                        return;
                    }

                    ERROR(fmt("Unsupported NOP mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));
                    
                    return;
                } break;

                case IM_NOT: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.opcode = HY_NOT;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_NOT;

                        return;
                    }

                    ERROR(fmt("Unsupported NOT mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_OR: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_ORR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_ORR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_ORI16;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYI8) {
                        m_instruction.opcode = HY_ORI8;

                        return;
                    }

                    ERROR(fmt("Unsupported OR mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_POP: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_POPS;

                        return;
                    };

                    if (m_instruction.mode == OP_RANGE) {
                        m_instruction.opcode = HY_POPM;

                        return;
                    }

                    ERROR(fmt("Unsupported POP mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;
                
                case IM_PUSH: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_PUSHS;

                        return;
                    };

                    if (m_instruction.mode == OP_RANGE) {
                        m_instruction.opcode = HY_PUSHM;

                        return;
                    }

                    ERROR(fmt("Unsupported PUSH mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_RET: {
                    if (m_instruction.mode == OP_NONE) {
                        m_instruction.opcode = HY_RETCC;
                        m_instruction.fieldx = 14;

                        return;
                    }

                    ERROR(fmt("Unsupported RET mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_RETCC: {
                    std::string cond = m_instruction.mnemonic.substr(m_instruction.mnemonic.size() - 2);
                    std::string uc = m_instruction.mnemonic;

                    for (char& c : uc) c = std::toupper(c);

                    if (m_instruction.mode == OP_NONE) {
                        m_instruction.opcode = HY_RETCC;
                        m_instruction.fieldx = m_cond_map[cond];

                        return;
                    }

                    ERROR(fmt("Unsupported %s mode %s", uc.c_str(), hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_RST: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.opcode = HY_RSTS;

                        return;
                    };

                    if (m_instruction.mode == OP_RANGE) {
                        m_instruction.opcode = HY_RSTM;

                        return;
                    }

                    ERROR(fmt("Unsupported RST mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_RTL: {
                    if (m_instruction.mode == OP_NONE) {
                        m_instruction.opcode = HY_RTLCC;
                        m_instruction.fieldx = 14;

                        return;
                    }

                    ERROR(fmt("Unsupported RTL mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_RTLCC: {
                    std::string cond = m_instruction.mnemonic.substr(m_instruction.mnemonic.size() - 2);
                    std::string uc = m_instruction.mnemonic;

                    for (char& c : uc) c = std::toupper(c);

                    if (m_instruction.mode == OP_NONE) {
                        m_instruction.opcode = HY_RTLCC;
                        m_instruction.fieldx = m_cond_map[cond];

                        return;
                    }

                    ERROR(fmt("Unsupported %s mode %s", uc.c_str(), hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_SEXT: {
                    if (m_instruction.mode == OP_RX) {
                        m_instruction.fieldy = m_instruction.fieldx;
                        m_instruction.opcode = HY_SEXT;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_SEXT;

                        return;
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for SEXT", m_instruction.specifier));

                    return;
                } break;

                case IM_STORE: {
                    if (m_instruction.specifier == ' ') { m_instruction.size = 2; }
                    else if (m_instruction.specifier == 'b') { m_instruction.size = 0; }
                    else if (m_instruction.specifier == 's') { m_instruction.size = 1; }
                    else if (m_instruction.specifier == 'l') { m_instruction.size = 2; }
                    else if (m_instruction.specifier == 'x') { m_instruction.size = 3; }
                    else {
                        ERROR(fmt("Unknown specifier \'%c\' for STORE", m_instruction.specifier));

                        return;
                    }

                    if (m_instruction.mode == OP_RXIND) {
                        m_instruction.opcode = m_instruction.shift ? HY_STORES : HY_STOREM;

                        return;
                    }

                    if (m_instruction.mode == OP_RXFIX) {
                        m_instruction.opcode = m_instruction.add ? HY_STOREFA : HY_STOREFS;

                        return;
                    }

                    ERROR(fmt("Unsupported STORE.%c mode %s", std::toupper(m_instruction.specifier), hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_SUB: {
                    if ((m_instruction.specifier == 'u') || (m_instruction.specifier == ' ')) {
                        // add.u r0, r1
                        if (m_instruction.mode == OP_RXRY) {
                            m_instruction.opcode = HY_SUBR;
                            m_instruction.fieldz = m_instruction.fieldy;
                            m_instruction.fieldy = m_instruction.fieldx;

                            return;
                        }

                        // add.u r0, r1, r2
                        if (m_instruction.mode == OP_RXRYRZ) {
                            m_instruction.opcode = HY_SUBR;

                            return;
                        }

                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_SUBUI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_SUBUI8;

                            return;
                        }

                        ERROR(fmt("Unsupported SUB.U mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    if (m_instruction.specifier == 's') {
                        if (m_instruction.mode == OP_RXI16) {
                            m_instruction.opcode = HY_SUBSI16;

                            return;
                        }

                        if (m_instruction.mode == OP_RXRYI8) {
                            m_instruction.opcode = HY_SUBSI8;

                            return;
                        }

                        ERROR(fmt("Unsupported SUB.S mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                        return;
                    }

                    ERROR(fmt("Unknown specifier \'%c\' for SUB", m_instruction.specifier));

                    return;
                } break;

                case IM_TST: {
                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_TST;

                        return;
                    }

                    ERROR(fmt("Unsupported TST mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;

                case IM_XOR: {
                    if (m_instruction.mode == OP_RXRY) {
                        m_instruction.opcode = HY_XORR;
                        m_instruction.fieldz = m_instruction.fieldy;
                        m_instruction.fieldy = m_instruction.fieldx;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYRZ) {
                        m_instruction.opcode = HY_XORR;

                        return;
                    }

                    if (m_instruction.mode == OP_RXI16) {
                        m_instruction.opcode = HY_XORI16;

                        return;
                    }

                    if (m_instruction.mode == OP_RXRYI8) {
                        m_instruction.opcode = HY_XORI8;

                        return;
                    }

                    ERROR(fmt("Unsupported XOR mode %s", hyrisc_mode_str[m_instruction.mode].c_str()));

                    return;
                } break;
            }
        }

        std::unordered_map <std::string, int> m_register_map = {
            { "r0" , 0  }, { "r1" , 1  }, { "r2" , 2  }, { "r3" , 3  },
            { "r4" , 4  }, { "r5" , 5  }, { "r6" , 6  }, { "r7" , 7  },
            { "r8" , 8  }, { "r9" , 9  }, { "r10", 10 }, { "r11", 11 },
            { "r12", 12 }, { "r13", 13 }, { "r14", 14 }, { "r15", 15 },
            { "r16", 16 }, { "r17", 17 }, { "r18", 18 }, { "r19", 19 },
            { "r20", 20 }, { "r21", 21 }, { "r22", 22 }, { "r23", 23 },
            { "r24", 24 }, { "r25", 25 }, { "r26", 26 }, { "r27", 27 },
            { "r28", 28 }, { "r29", 29 }, { "sp" , 30 }, { "pc" , 31 },
            { "r30", 30 }, { "r31", 31 },
            // hs ABI
            { "a0" , 24 }, { "a1" , 25 }, { "a2" , 26 }, { "a3" , 27 },
            { "tr" , 28 }, { "fp" , 29 }
        };

        void parse_mnemonic(std::string mnemonic) {
            auto dp = mnemonic.find('.');

            if (dp == std::string::npos) {
                m_instruction.mnemonic = mnemonic;
                m_instruction.specifier = ' ';
            } else {
                m_instruction.mnemonic = mnemonic.substr(0, dp);
                m_instruction.specifier = mnemonic.at(dp + 1);
            }
        }

        std::string parse_name() {
            if (!(std::isalpha(m_current) || m_current == '_')) {
                // Error expected name

                return "";
            }

            std::string name;

            name.push_back(m_current);

            m_current = m_input->get();

            while (std::isalnum(m_current) || (m_current == '_') || (m_current == '.')) {
                name.push_back(m_current);

                m_current = m_input->get();
            }

            return name;
        }

        uint32_t bin_stoul(const std::string& bin) {
            uint32_t value = 0;

            for (int i = 0; i < bin.size(); i++)
                value |= (bin[i] == '1' ? 1 : 0) << i;
            
            return value;
        }

        uint32_t parse_integer() {
            bool negative = false;

            if (m_current == '-') {
                negative = true;

                m_current = m_input->get();
            }

            std::string integer;
            uint32_t value;

            // Parse numeric constant
            if (std::isdigit(m_current)) {
                integer.push_back(m_current);

                m_current = m_input->get();

                // Parse binary number
                if (m_current == 'b') {
                    m_current = m_input->get();

                    integer.clear();

                    while ((m_current == '1') || (m_current == '0')) {
                        integer.push_back(m_current);

                        m_current = m_input->get();
                    }

                    value = bin_stoul(integer);

                    return negative ? -value : value;
                } else if ((m_current == 'x') || std::isdigit(m_current)) {
                    // Parse hex or dec
                    integer.push_back(m_current);

                    m_current = m_input->get();

                    while (std::isxdigit(m_current) || std::isdigit(m_current)) {
                        integer.push_back(m_current);

                        m_current = m_input->get();
                    }

                    value = std::stoul(integer, nullptr, 0);

                    return negative ? -value : value;
                } else {
                    value = std::stoul(integer, nullptr, 0);

                    return negative ? -value : value;
                }
            }

            // Parse char constant
            if (m_current == '\'') {
                m_current = m_input->get();

                value = negative ? -(int)(m_current) : m_current;

                // Consume char
                m_current = m_input->get();

                if (m_current != '\'') {
                    ERROR("Expected closing \' after char constant");

                    return 0;
                }

                // Consume closing '
                m_current = m_input->get();

                return value;
            }

            return value;
        }

        void parse_register_range() {
            if (m_current != '{') {
                // Error expected opening brace

                return;
            }

            m_current = m_input->get();

            std::string start = parse_name();

            if (!m_register_map.contains(start)) {
                // Error expected register

                return;
            }

            m_instruction.fieldx = m_register_map[start];

            ignore_whitespace();

            switch (m_current) {
                case ',': case '-': break;

                default: {
                    // Error expected , or -

                    return;
                } break;
            }

            m_current = m_input->get();

            ignore_whitespace();

            std::string end = parse_name();

            if (!m_register_map.contains(end)) {
                // Error expected register

                return;
            }

            m_instruction.fieldy = m_register_map[end];

            ignore_whitespace();

            if (m_current != '}') {
                // Error expected closing brace

                return;
            }

            m_current = m_input->get();

            return;
        }

        void parse_indexed_operand() {
            bool add = false;
            bool fixed = false;
            bool shift = false;

            if (m_current != '[') {
                // Error expected opening bracket

                return;
            }

            m_current = m_input->get();

            std::string br = parse_name();

            if (!m_register_map.contains(br)) {
                // Error expected register
                return;
            }

            m_instruction.fieldy = m_register_map[br];

            ignore_whitespace();

            switch (m_current) {
                case '+': {
                    m_instruction.add = true;
                } break;

                case '-': {
                    m_instruction.add = false;
                } break;

                case ']': {
                    m_instruction.add = true;

                    // We're done

                    m_current = m_input->get();

                    return;
                }

                default: {
                    // Error expected operator
                    return;
                } break;
            }

            m_current = m_input->get();

            ignore_whitespace();

            if (std::isalpha(m_current) || m_current == '_') {
                // Indexed operand isn't fixed offset

                if (!m_instruction.add) {
                    ERROR("Indexed register mode doesn't support subtraction");

                    return;
                }

                std::string ir = parse_name();

                if (!m_register_map.contains(ir)) {
                    ERROR(fmt("Expected register, got %s instead", ir.c_str()));

                    return;
                }

                m_instruction.fieldz = m_register_map[br];

                ignore_whitespace();

                switch (m_current) {
                    case '*': { m_instruction.shift = false; } break;
                    case ':': { m_instruction.shift = true; } break;
                    case ']': {
                        // We're done, set shift to false and shiftmul to 1
                        m_instruction.shift = false;
                        m_instruction.fieldw = 1;

                        m_current = m_input->get();

                        return;
                    } break;
                }

                m_current = m_input->get();

                ignore_whitespace();

                if (!(std::isdigit(m_current) || m_current == '\'' || m_current == '-')) {
                    ERROR("Expected immediate");

                    return;
                }

                m_instruction.fieldw = parse_integer();

                ignore_whitespace();

                if (m_current != ']') {
                    ERROR(fmt("Expected closing bracket, got %c instead", m_current));

                    return;
                }

                m_current = m_input->get();

                return;
            } else if (std::isdigit(m_current) || m_current == '\'' || m_current == '-') {
                // Indexed operand is fixed offset
                m_instruction.mode = OP_RXFIX;

                uint32_t offset = parse_integer();

                m_instruction.fieldz = offset & 0x1f;
                m_instruction.fieldw = (offset >> 5) & 0x1f;

                ignore_whitespace();

                if (m_current != ']') {
                    // Error expected closing bracket

                    return;
                }

                m_current = m_input->get();

                return;
            } else {
                // Error expected register or immediate

                return;
            }
        }

        void parse_operand() {
            if (std::isalpha(m_current) || m_current == '_') {
                std::string name = parse_name();

                // Operand is a register
                if (m_register_map.contains(name)) {
                    int rn = m_register_map[name];

                    switch (m_instruction.mode) {
                        case OP_NONE: {
                            m_instruction.fieldx = rn;
                            m_instruction.mode = OP_RX;
                        } break;

                        case OP_RX: {
                            m_instruction.fieldy = rn;
                            m_instruction.mode = OP_RXRY;
                        } break;
                        
                        case OP_RXRY: {
                            m_instruction.fieldz = rn;
                            m_instruction.mode = OP_RXRYRZ;
                        } break;

                        case OP_INDEX: {
                            m_instruction.fieldx = rn;
                            m_instruction.mode = OP_RXIND;
                        } break;
                    }
                } else {
                    uint32_t value = 0;
                    bool found = false;

                    // Operand is a name
                    if (m_symbol_map.contains(name)) {
                        value = m_symbol_map[name];
                        found = true;
                    }

                    if (m_local_map.contains(name)) {
                        value = m_local_map[name] - m_pos;

                        found = true;
                    }

                    if ((!found) && (m_pass == 1)) {
                        ERROR(fmt("Couldn't find symbol %s", name.c_str()));

                        return;
                    }

                    switch (m_instruction.mode) {
                        case OP_NONE: {
                            m_instruction.imm16 = value;
                            m_instruction.mode = OP_I16;
                        } break;
                        
                        case OP_RX: {
                            m_instruction.imm16 = value;
                            m_instruction.mode = OP_RXI16;
                        } break;

                        case OP_RXRY: {
                            m_instruction.imm8 = value;
                            m_instruction.mode = OP_RXRYI8;
                        } break;
                    }
                }
            }

            // Operand is a numeric constant
            if (std::isdigit(m_current) || m_current == '\'' || m_current == '-') {
                uint32_t value = parse_integer();

                switch (m_instruction.mode) {
                    case OP_NONE: {
                        m_instruction.imm16 = value;
                        m_instruction.mode = OP_I16;
                    } break;
                    
                    case OP_RX: {
                        m_instruction.imm16 = value;
                        m_instruction.mode = OP_RXI16;
                    } break;

                    case OP_RXRY: {
                        m_instruction.imm8 = value;
                        m_instruction.mode = OP_RXRYI8;
                    } break;
                }
            }

            if (m_current == '[') {
                parse_indexed_operand();

                switch (m_instruction.mode) {
                    case OP_NONE: {
                        m_instruction.mode = OP_INDEX;
                    } break;

                    case OP_RX: {
                        m_instruction.mode = OP_RXIND;
                    } break;
                }
            }

            if (m_current == '{') {
                parse_register_range();

                switch (m_instruction.mode) {
                    case OP_NONE: {
                        m_instruction.mode = OP_RANGE;
                    } break;
                }
            }
        }

        bool isoperand() {
            return std::isalpha(m_current) ||
                   (m_current == '_') ||
                   std::isdigit(m_current) ||
                   (m_current == '-') ||
                   (m_current == '\'') ||
                   (m_current == '[') ||
                   (m_current == '{');
        }

        void consume_until_eol() {
            while (m_current != '\n') {
                m_current = m_input->get();
            }

            m_current = m_input->get();
        }

        void ignore_whitespace() {
            while (std::isspace(m_current)) {
                m_current = m_input->get();
            }
        }

        uint32_t encode_instruction() {
            m_instruction.encoding = hyrisc_mode_encoding[m_instruction.mode];

            uint32_t opcode   = (m_instruction.opcode & 0xff);
            uint32_t encoding = (m_instruction.encoding & 0x3) << 8;
            uint32_t fieldx   = (m_instruction.fieldx & 0x1f) << 10;
            uint32_t fieldy   = (m_instruction.fieldy & 0x1f) << 15;
            uint32_t fieldz   = (m_instruction.fieldz & 0x1f) << 20;
            uint32_t fieldw   = (m_instruction.fieldw & 0x1f) << 25;
            uint32_t size     = (m_instruction.size & 0x3) << 30;
            uint32_t imm8     = (m_instruction.imm8 & 0xff) << 20;
            uint32_t imm16    = (m_instruction.imm16 & 0xffff) << 15;

            uint32_t encoded = 0;

            switch (m_instruction.encoding) {
                case ENC_4: { encoded = opcode | encoding | fieldx | fieldy | fieldz | fieldw | size; } break;
                case ENC_3: { encoded = opcode | encoding | fieldx | fieldy | imm8; } break;
                case ENC_2: { encoded = opcode | encoding | fieldx | imm16; } break;
                case ENC_1: { ERROR("Encoding 0 (1) is unsupported"); } break;
            }

            // _log(debug, "mode=%i, opcode=%02x, encoding=%u, fieldx=%04x, fieldy=%04x, fieldz=%04x, fieldw=%04x, size=%04x, imm8=%02x, imm16=%04x, encoded=%08x",
            //     m_instruction.mode,
            //     m_instruction.opcode,
            //     m_instruction.encoding,
            //     m_instruction.fieldx,
            //     m_instruction.fieldy,
            //     m_instruction.fieldz,
            //     m_instruction.fieldw,
            //     m_instruction.size,
            //     m_instruction.imm8,
            //     m_instruction.imm16,
            //     encoded
            // );

            return encoded;
        }

        void write_instruction(uint32_t insn) {
            uint32_t opcode = insn;

            m_output->write((char*)&opcode, sizeof(uint32_t));
        }

        enum directive_t {
            AD_LOAD32,
            AD_ELF_TEXT
        };

        std::unordered_map <std::string, directive_t> m_directive_map = {
            { "load32", AD_LOAD32 },
            { "text", AD_ELF_TEXT }
        };

        void handle_load32() {
            std::string reg = parse_name();

            if (!m_register_map.contains(reg)) {
                ERROR(fmt("Expected register, got %s", reg.c_str()));

                return;
            }

            ignore_whitespace();

            uint32_t value;

            // Operand is an integer
            if (std::isdigit(m_current) || m_current == '-' || m_current == '\'') {
                switch (m_pass) {
                    case 0: {
                        value = parse_integer();

                        m_pos += (value & 0xffff0000) ? 8 : 4;
                    } break;

                    case 1: {
                        uint32_t opcode = 0;

                        value = parse_integer();

                        if (!value) {
                            m_instruction.opcode = HY_RSTS;
                            m_instruction.mode   = OP_RX;
                            m_instruction.fieldx = m_register_map[reg];

                            opcode = encode_instruction();

                            write_instruction(encode_instruction());

                            m_pos += 4;
                        } else if (value & 0xffff0000) {
                            m_instruction.opcode = HY_LUI;
                            m_instruction.mode   = OP_RXI16;
                            m_instruction.fieldx = m_register_map[reg];
                            m_instruction.imm16  = (value >> 16) & 0xffff;

                            write_instruction(encode_instruction());

                            m_instruction.opcode = HY_ORI16;
                            m_instruction.mode   = OP_RXI16;
                            m_instruction.fieldx = m_register_map[reg];
                            m_instruction.imm16  = value & 0xffff;

                            write_instruction(encode_instruction());

                            m_pos += 8;
                        } else {
                            m_instruction.opcode = HY_LI;
                            m_instruction.mode   = OP_RXI16;
                            m_instruction.fieldx = m_register_map[reg];
                            m_instruction.imm16  = value & 0xffff;

                            write_instruction(encode_instruction());

                            m_pos += 4;
                        }
                        
                    } break;
                }
            } else {
                // This is a name we have to look up
                switch (m_pass) {
                    case 0: {
                        m_pos += 8;
                    } break;

                    case 1: {
                        std::string symbol = parse_name();

                        // _log(debug, "symbol=%s", symbol.c_str());

                        if (m_local_map.contains(symbol)) {
                            value = m_local_map[symbol] - m_pos;

                            goto found;
                        }

                        if (m_symbol_map.contains(symbol)) {
                            value = m_symbol_map[symbol];

                            goto found;
                        }

                        ERROR(fmt("Symbol \"%s\" not found", symbol.c_str()));

                        return;

                        found:

                        uint32_t opcode = 0;

                        m_instruction.opcode = HY_LUI;
                        m_instruction.mode   = OP_RXI16;
                        m_instruction.fieldx = m_register_map[reg];
                        m_instruction.imm16  = (value >> 16) & 0xffff;

                        opcode = encode_instruction();

                        m_output->write((char*)&opcode, sizeof(opcode));
                        
                        m_instruction.opcode = HY_ORI16;
                        m_instruction.mode   = OP_RXI16;
                        m_instruction.fieldx = m_register_map[reg];
                        m_instruction.imm16  = value & 0xffff;

                        opcode = encode_instruction();

                        m_output->write((char*)&opcode, sizeof(opcode));

                        m_pos += 8;
                    } break;
                }
            }
        }

        void handle_directives() {
            // Consume .
            m_current = m_input->get();

            std::string str = parse_name();

            if (!m_directive_map.contains(str)) {
                ignore_whitespace();

                if (m_current != ':') {
                    ERROR("Expected : after local label definition");

                    return;
                }

                m_local_map.insert({str, m_pos});

                m_current = m_input->get();

                return;
            }

            directive_t directive = m_directive_map[str];

            ignore_whitespace();

            switch (directive) {
                case AD_LOAD32: { handle_load32(); } break;
            }

            consume_until_eol();

            return;
        }

        void parse_line() {
            if (m_input->eof()) return;

            m_instruction.encoding  = 0;
            m_instruction.fieldx    = 0;
            m_instruction.fieldy    = 0;
            m_instruction.fieldz    = 0;
            m_instruction.fieldw    = 0;
            m_instruction.imm16     = 0;
            m_instruction.imm8      = 0;
            m_instruction.mode      = OP_NONE;
            m_instruction.opcode    = 0;
            m_instruction.shift     = false;
            m_instruction.specifier = 0;
            m_instruction.mnemonic.clear();

            ignore_whitespace();

            if (m_current == '.') {
                handle_directives();

                return;
            }

            std::string name = parse_name();

            while (isblank(m_current)) {
                m_current = m_input->get();
            }

            if (m_current == ':') {
                m_local_map.clear();

                m_symbol_map.insert({name, m_pos});

                // Consume :
                m_current = m_input->get();

                parse_line();

                return;
            } else if (isoperand()) {
                parse_mnemonic(name);
                parse_operand();

                while (isblank(m_current)) {
                    m_current = m_input->get();
                }

                while (m_current == ',') {
                    m_current = m_input->get();

                    while (isblank(m_current)) {
                        m_current = m_input->get();
                    }

                    parse_operand();

                    while (isblank(m_current)) {
                        m_current = m_input->get();
                    }
                }

                parse_opcode();

                switch (m_pass) {
                    case 0: {
                        m_pos += 4;
                    } break;

                    case 1: {
                        write_instruction(encode_instruction());

                        m_pos += 4;
                    } break;
                }

                return;
            } else if (std::isspace(m_current) || (m_current == -1)) {
                parse_mnemonic(name);

                ignore_whitespace();

                parse_opcode();

                switch (m_pass) {
                    case 0: {
                        m_pos += 4;
                    } break;

                    case 1: {
                        write_instruction(encode_instruction());

                        m_pos += 4;
                    } break;
                }

                return;
            }
        }
    
    public:
        void init(std::istream* input, std::ostream* output, error_logger_t* logger) {
            m_input = input;
            m_output = output;
            m_logger = logger;

            m_current = m_input->get();
        }

        void assemble() {
            m_pass = 0;

            while (!m_input->eof()) {
                parse_line();
            }

            m_input->clear();
            m_input->seekg(0);
            
            m_current = m_input->get();

            m_pos = 0;
            m_pass = 1;

            while (!m_input->eof()) {
                parse_line();
            }
        }
    };
}

#undef ERROR