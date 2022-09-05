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

    class assembler_t {
        std::istream* m_input;
        char m_current;
        error_logger_t* m_logger;
        std::vector <uint8_t> m_output;
        std::unordered_map <std::string, uint32_t> m_symbol_map;
    
        uint32_t m_pos;

#include "instructions.hpp"
    
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

        // uint32_t encode_instruction() {
        //     encoding_t encoding = m_mode_encoding[m_instruction.mode];

        //     switch (encoding) {
        //         case ENC_4: {

        //         }
        //     }
        // }

        void parse_line() {
            ignore_whitespace();

            if (m_current == '.') {
                // Assembler directive

                return;
            }

            std::string name = parse_name();

            ignore_whitespace();

            if (m_current == ':') {
                m_symbol_map.insert({name, m_pos});
            } else if (std::isalpha(m_current)) {

            }
        }
    };
}