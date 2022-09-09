#pragma once

#include <unordered_map>
#include <iostream>
#include <string>

#include "../../error.hpp"

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
    
        uint32_t m_pos;

#include "instructions.hpp"

        struct instruction_t {
            std::string mnemonic;
            operand_mode_t mode;
            char specifier;

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
                m_instruction.mnemonic = mnemonic.substr(dp);
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

        void parse_operands() {
            if (std::isalpha(m_current) || m_current == '_') {
                std::string name = parse_name();

                if (m_register_map.contains(name)) {
                    _log(debug, "R%u", m_register_map[name]);
                }
            }
        }
    
    public:
        void init(std::istream* input, error_logger_t* logger) {
            m_input = input;
            m_logger = logger;

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

            ignore_whitespace();

            if (m_current == ':') {
                m_local_map.clear();
                m_symbol_map.insert({name, m_pos});

                _log(debug, "label=%s", name.c_str());

                // Consume :
                m_current = m_input->get();

                parse_line();
            } else if (std::isalpha(m_current)) {
                _log(debug, "mnemonic=%s, current=%c", name.c_str(), m_current);

                parse_mnemonic(name);

                consume_until_eol();
            }
        }

        void assemble() {
            while (!m_input->eof()) {
                parse_line();
            }
        }
    };
}