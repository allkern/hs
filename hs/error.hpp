#pragma once

#include "log.hpp"

#include <vector>
#include <string>

namespace hs {
    template <class... Args> static std::string fmt(std::string fmt, Args... args) {
        char* buf = new char[fmt.size() + 0x100];

        std::sprintf(buf, fmt.c_str(), args...);

        std::string str(buf);

        return str;
    }

    struct error_logger_t {
        std::vector <std::string> m_source;
        std::string m_filename = "";

        std::string get_error_highlighted_string(std::string str, int start, int end) {
            return str.substr(0, start) + ESCAPE(31;1) + str.substr(start, end - start) + ESCAPE(0) + str.substr(end);
        }

        std::string get_warning_highlighted_string(std::string str, int start, int end) {
            return str.substr(0, start) + ESCAPE(35;1) + str.substr(start, end - start) + ESCAPE(0) + str.substr(end);
        }

    public:
        void init(std::istream* input, std::string filename = "") {
            while (!input->eof()) {
                std::string line;

                std::getline(*input, line);

                m_source.push_back(line);
            }

            m_filename = filename;
            
            input->clear();
            input->seekg(0);
        }

        void print_warning(std::string module, std::string warn, int line, int col, int len, bool print_hint) {
            if (m_filename.size()) {
                _log(warning, "in " ESCAPE(37;1) "%s: " ESCAPE(0) "%s: %s (at " ESCAPE(37;1) "L%u" ESCAPE(0) ", " ESCAPE(37;1) "C%u" ESCAPE(0) ")",
                    m_filename.c_str(),
                    module.c_str(),
                    warn.c_str(),
                    line + 1,
                    col + 1
                );
            } else {
                _log(warning, "%s: %s (at %u;%u)",
                    module.c_str(),
                    warn.c_str(),
                    line + 1,
                    col + 1
                );
            }

            if (!print_hint) return;

            std::string highlighted = get_warning_highlighted_string(m_source[line], col, col + len);
            std::string marker;

            if (col) marker = std::string(col, ' ');

            marker.append(ESCAPE(35;1) "^");

            if (len > 1) marker.append(std::string(len - 1, '~'));

            marker.append(ESCAPE(0));

            _log(info, "here:\n  %u\t| %s\n  %u\t| %s",
                line + 1,
                highlighted.c_str(),
                line + 2,
                marker.c_str()
            );
        }

        void print_error(std::string module, std::string err, int line, int col, int len, bool print_hint = true, bool without_loc_info = false) {
            if (m_filename.size()) {
                if (!without_loc_info) { 
                    _log(error, "in " ESCAPE(37;1) "%s: " ESCAPE(0) "%s: %s (at " ESCAPE(37;1) "L%u" ESCAPE(0) ", " ESCAPE(37;1) "C%u" ESCAPE(0) ")",
                        m_filename.c_str(),
                        module.c_str(),
                        err.c_str(),
                        line + 1,
                        col + 1
                    );
                } else {
                    _log(error, "in " ESCAPE(37;1) "%s: " ESCAPE(0) "%s: %s",
                        m_filename.c_str(),
                        module.c_str(),
                        err.c_str()
                    );
                }
            } else {
                if (!without_loc_info) {
                    _log(error, "%s: %s (at %u;%u)",
                        module.c_str(),
                        err.c_str(),
                        line + 1,
                        col + 1
                    );
                } else {
                    _log(error, ESCAPE(37;1) "%s: " ESCAPE(0) "%s",
                        module.c_str(),
                        err.c_str()
                    );
                }
            }

            if (!print_hint) return;

            std::string highlighted = get_error_highlighted_string(m_source[line], col, col + len);
            std::string marker;

            if (col) marker = std::string(col, ' ');

            marker.append(ESCAPE(31;1) "^");

            if (len > 1) marker.append(std::string(len - 1, '~'));

            marker.append(ESCAPE(0));

            _log(info, "here:\n  %u\t| %s\n  %u\t| %s",
                line + 1,
                highlighted.c_str(),
                line + 2,
                marker.c_str()
            );
        }
    };
}