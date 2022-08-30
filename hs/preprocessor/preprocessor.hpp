
#pragma once

#include "../error.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>

namespace hs {
    class preprocessor_t {
        std::istream* m_input;
        std::stringstream m_output;
        error_logger_t* m_logger;

        char m_current;

        enum directive_t {
            PD_NONE,
            PD_INCLUDE,
            PD_DEFINE,
            PD_UNDEF,
            PD_MACRO
        };

        std::unordered_map <std::string, directive_t> m_directive_map = {
            { "include", PD_INCLUDE },
            { "define" , PD_DEFINE  },
            { "undef"  , PD_UNDEF   },
            { "macro"  , PD_MACRO   }
        };

        inline void ignore_whitespace(bool output = true) {
            while (isspace(m_current)) {
                if (output) m_output.put(m_current);

                m_current = m_input->get();
            }
        }

        std::string parse_string() {
            if (m_current != '\"') return "";

            std::string str;

            m_current = m_input->get();

            while (m_current != '\"') {
                str.push_back(m_current);

                m_current = m_input->get();
            }

            m_current = m_input->get();

            return str;
        }

        directive_t parse_directive() {
            if (!std::isalpha(m_current)) {
                return PD_NONE;
            }

            std::string directive_text;

            while (std::isalpha(m_current)) {
                directive_text.push_back(m_current);

                m_current = m_input->get();
            }

            if (!m_directive_map.contains(directive_text)) {
                return PD_NONE;
            }

            return m_directive_map[directive_text];
        }

        bool parse_include_directive() {
            ignore_whitespace(false);

            std::string filename = parse_string();

            if (!filename.size()) {
                // Error :P

                return false;
            }

            std::ifstream file(filename, std::ios::binary);

            if (!file.is_open() || !file.good()) {
                if (m_logger) m_logger->print_error(
                    "preprocessor",
                    fmt("File \"%s\" for include wasn't found", filename.c_str()), 0, 0, false
                );

                return false;
            }

            preprocessor_t include;

            include.init(&file, m_logger);
            include.preprocess();
            
            auto file_output = include.get_output();

            char c = file_output->get();

            while (!file_output->eof()) {
                // Don't output the EOF char from the included file
                if (!iseof(c)) m_output.put(c);

                c = file_output->get();
            }
        }

        inline bool iseof(char c) {
            return c == -1;
        }

        inline bool isnewline(char c) {
            return (c == '\n') || (c == '\r');
        }

        void ignore_until_eol(bool output) {
            while ((!isnewline(m_current)) && !m_input->eof()) {
                if (output) m_output.put(m_current);

                m_current = m_input->get();
            }

            while (isnewline(m_current)) {
                if (output) m_output.put(m_current);

                m_current = m_input->get();
            }

            if (m_current != -1) m_output.put(m_current);
        }
    
    public:
        void init(std::istream* input, error_logger_t* logger) {
            m_input = input;
            m_logger = logger;

            m_current = m_input->get();
        }

        std::stringstream* get_output() {
            return &m_output;
        }

        std::string m_name;

        void preprocess() {
            while (!m_input->eof()) {
                // Handle preprocessor directive or comment
                if (m_current == '#') {
                    m_current = m_input->get();

                    directive_t directive = parse_directive();

                    switch (directive) {
                        case PD_INCLUDE: {
                            if (!parse_include_directive()) break;
                        } break;
                    }

                    consume_rest_of_line:

                    while ((!isnewline(m_current)) && (!iseof(m_current))) {
                        m_current = m_input->get();
                    }

                    // We could discard newlines, but this would mismatch processed
                    // line numbers vs source line numbers, also inline comments
                    // would screw the next line

                    // Handle LF or CRLF
                    // if (isnewline(m_current)) m_current = m_input->get();
                    // if (isnewline(m_current)) m_current = m_input->get();
                } else if (std::isalpha(m_current) || (m_current == '_')) {
                    // Handle name stuff
                    m_name.push_back(m_current);

                    m_current = m_input->get();

                    while (std::isalpha(m_current) || (m_current == '_') || std::isdigit(m_current)) {
                        m_name.push_back(m_current);

                        m_current = m_input->get();
                    }

                    // if is preprocessor token then replace else

                    for (char c : m_name)
                        m_output.put(c);
                    
                    m_name.clear();
                } else {
                    m_output.put(m_current);

                    m_current = m_input->get();
                }
            }
        }
    };
}