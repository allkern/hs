
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

        std::unordered_map <std::string, std::string> m_define_map;

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

            // Get defines from the processed file and accumulate
            // them into our own defines
            auto* map = include.get_define_map();

            for (auto& p : *map) {
                m_define_map.insert(p);
            }
            
            auto file_output = include.get_output();

            char c = file_output->get();

            while (!file_output->eof()) {
                // Don't output the EOF char from the included file
                if (!iseof(c)) m_output.put(c);

                c = file_output->get();
            }

            return true;
        }

        const std::string WHITESPACE = " \n\r\t\f\v";

        std::string ltrim(const std::string &s) {
            size_t start = s.find_first_not_of(WHITESPACE);

            return (start == std::string::npos) ? "" : s.substr(start);
        }
        
        std::string rtrim(const std::string &s) {
            size_t end = s.find_last_not_of(WHITESPACE);

            return (end == std::string::npos) ? "" : s.substr(0, end + 1);
        }
        
        std::string trim(const std::string &s) {
            return rtrim(ltrim(s));
        }

        bool parse_undef_directive() {
            ignore_whitespace(false);

            if (!(std::isalpha(m_current) || (m_current == '_')))
                return false;

            std::string name;

            name.push_back(m_current);

            m_current = m_input->get();

            while (std::isalpha(m_current) || (m_current == '_') || std::isdigit(m_current)) {
                name.push_back(m_current);

                m_current = m_input->get();
            }

            if (m_define_map.contains(name)) {
                m_define_map.erase(name);
            }

            return true;
        }

        bool parse_define_directive() {
            ignore_whitespace(false);

            if (!(std::isalpha(m_current) || (m_current == '_')))
                return false;

            std::string name, value;

            name.push_back(m_current);

            m_current = m_input->get();

            while (std::isalpha(m_current) || (m_current == '_') || std::isdigit(m_current)) {
                name.push_back(m_current);

                m_current = m_input->get();
            }

            ignore_whitespace(false);

            while ((!isnewline(m_current)) && !m_input->eof()) {
                value.push_back(m_current);

                m_current = m_input->get();
            }

            value = trim(value);

            m_define_map.insert({name, value});

            _log(debug, "name=%s, value=%s", name.c_str(), value.c_str());

            return true;
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
        std::unordered_map <std::string, std::string>* get_define_map() {
            return &m_define_map;
        }

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
                        
                        case PD_DEFINE: {
                            if (!parse_define_directive()) break;
                        } break;

                        case PD_UNDEF: {
                            if (!parse_undef_directive()) break;
                        } break;
                    }

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

                    if (m_define_map.contains(m_name)) {
                        m_name = m_define_map[m_name];
                    }

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