#pragma once

#include <unordered_map>
#include <iostream>
#include <string>

#include "../../error.hpp"

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "assembler", \
        msg, \
        0, 0, 0, false, true \
    );

namespace hs {
    enum output_format_t {
        F_ELF32,
        F_RAW
    };

    class hyrisc_assembler_t {
        std::istream* m_input;
        char m_current;
        error_logger_t* m_logger;
        std::vector <uint8_t> m_output;
        std::unordered_map <std::string, uint32_t> m_symbol_map;
        std::unordered_map <std::string, uint32_t> m_local_map;

        int m_pass = 0;
    
        uint32_t m_pos;

#include "instructions.hpp"

        struct instruction_t {
            std::string mnemonic;
            operand_mode_t mode;
            char specifier;
            bool shift = false;

            // Data actually on opcode
            uint8_t  encoding;
            uint8_t  opcode;
            uint16_t fieldx, fieldy, fieldz, fieldw;
            uint8_t  imm8;
            uint16_t imm16;
        } m_instruction;

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
                    value = negative ? -value : value;
                }

                // Parse hex or dec
                if ((m_current == 'x') || std::isdigit(m_current)) {
                    integer.push_back(m_current);

                    m_current = m_input->get();

                    while (std::isxdigit(m_current) || std::isdigit(m_current)) {
                        integer.push_back(m_current);

                        m_current = m_input->get();
                    }

                    value = std::stoul(integer, nullptr, 0);
                    value = negative ? -value : value;
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
                    add = true;
                } break;
                case '-': {
                    add = false;
                } break;
                case ']': {
                    add = true;

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

                if (!add) {
                    // Error indexed register mode doesn't support subtracttion mode

                    return;
                }

                std::string ir = parse_name();

                if (!m_register_map.contains(ir)) {
                    // Error expected register
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
                    // Error expected immediate

                    return;
                }

                m_instruction.fieldw = parse_integer();

                ignore_whitespace();

                if (m_current != ']') {
                    // Error expected closing bracket
                    return;
                }

                m_current = m_input->get();

                return;
            } else if (std::isdigit(m_current) || m_current == '\'' || m_current == '-') {
                // Indexed operand is fixed offset
                m_instruction.mode = add ? OP_RXFIXA : OP_RXFIXS;

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
                        value = m_local_map[name];
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
    
    public:
        void init(std::istream* input, error_logger_t* logger) {
            m_input = input;
            m_logger = logger;

            m_instruction.mode = OP_NONE;

            m_current = m_input->get();
        }

        void ignore_whitespace() {
            while (std::isspace(m_current)) {
                m_current = m_input->get();
            }
        }

        // uint32_t encode_instruction() {
        //     encoding_t encoding = m_mode_encoding[m_instruction.mode];

        //     switch (encoding) {
        //         case ENC_4: {

        //         }
        //     }
        // }

        void consume_until_eol() {
            while (m_current != '\n') {
                m_current = m_input->get();
            }

            m_current = m_input->get();
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
                // Assembler directive
                m_current = m_input->get();

                std::string directive = parse_name();

                _log(debug, "directive=%s", directive.c_str());

                consume_until_eol();

                return;

                ignore_whitespace();

                m_local_map.insert({directive, m_pos});

                return;
            }

            std::string name = parse_name();

            while (isblank(m_current)) {
                m_current = m_input->get();
            }

            if (m_current == ':') {
                m_local_map.clear();
                m_symbol_map.insert({name, m_pos});

                _log(debug, "label=%s", name.c_str());

                // Consume :
                m_current = m_input->get();

                parse_line();
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

                static std::string mode_str[] = {
                    "OP_RX",
                    "OP_RXRY",
                    "OP_RXI16",
                    "OP_RXRYRZ",
                    "OP_RXRYI8",
                    "OP_RXIND",
                    "OP_RXFIXA",
                    "OP_RXFIXS",
                    "OP_I16",
                    "OP_INDEX",
                    "OP_RANGE",
                    "OP_NONE" 
                };

                _log(debug, "mnemonic=%s, specifier=%c, mode=%s",
                    m_instruction.mnemonic.c_str(),
                    m_instruction.specifier,
                    mode_str[(int)m_instruction.mode].c_str()
                );

                return;
            } else if (std::isspace(m_current)) {
                parse_mnemonic(name);
                ignore_whitespace();

                static std::string mode_str[] = {
                    "OP_RX",
                    "OP_RXRY",
                    "OP_RXI16",
                    "OP_RXRYRZ",
                    "OP_RXRYI8",
                    "OP_RXIND",
                    "OP_RXFIXA",
                    "OP_RXFIXS",
                    "OP_I16",
                    "OP_INDEX",
                    "OP_RANGE",
                    "OP_NONE" 
                };

                _log(debug, "mnemonic=%s, specifier=%c, mode=%s",
                    m_instruction.mnemonic.c_str(),
                    m_instruction.specifier,
                    mode_str[(int)m_instruction.mode].c_str()
                );

                return;
            }
        }

        void assemble() {
            while (!m_input->eof()) {
                parse_line();
            }
        }
    };
}

#undef ERROR