#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "error.hpp"

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "cli", \
        msg, \
        0, 0, 0, false, true \
    ); \
    return false;

namespace hs {
    enum cli_switch_t {
        SW_VERSION,
        SW_QUIET,
        SW_VERBOSE,
        SW_ASSEMBLE,
        SW_OUTPUT_ASSEMBLY,
        SW_LOG,
        SW_HELP,
        SW_ONLY_SYMBOLS,
        SW_DEBUG_LEXER_OUTPUT,
        SW_DEBUG_PARSER_OUTPUT,
        SW_DEBUG_IR_OUTPUT,
        SW_DEBUG_IRT_OUTPUT,
        SW_DEBUG_ALL
    };

    enum cli_setting_t {
        ST_INCLUDE_PATH,
        ST_INPUT,
        ST_OUTPUT,
        ST_OUTPUT_FORMAT,
        ST_OUTPUT_SYMBOLS,
        ST_XLAT,
        ST_HELP_TARGET
    };

    class cli_parser_t {
        int m_argc = 0;

        const char** m_argv = nullptr;

        error_logger_t* m_logger = nullptr;

        std::unordered_map <cli_setting_t, std::string> m_settings;
        std::unordered_map <cli_switch_t , bool>        m_switches;

#define WSHORTHAND(shortname, longname, st) \
    { shortname, st }, \
    { longname , st }

#define LONG_ONLY(longname, st) { longname, st }

        std::unordered_map <std::string, cli_switch_t> m_switches_map = {
            WSHORTHAND("-v", "--version"             , SW_VERSION            ),
            WSHORTHAND("-q", "--quiet"               , SW_QUIET              ),
            WSHORTHAND("-V", "--verbose"             , SW_VERBOSE            ),
            WSHORTHAND("-a", "--assemble"            , SW_ASSEMBLE           ),
            WSHORTHAND("-L", "--log"                 , SW_LOG                ),
            WSHORTHAND("-A", "--output-assembly"     , SW_OUTPUT_ASSEMBLY    ),
            LONG_ONLY (      "--help"                , SW_HELP               ),
            LONG_ONLY (      "--only-symbols"        , SW_ONLY_SYMBOLS       ),
            LONG_ONLY (      "--debug-lexer-output"  , SW_DEBUG_LEXER_OUTPUT ),
            LONG_ONLY (      "--debug-parser-output" , SW_DEBUG_PARSER_OUTPUT),
            LONG_ONLY (      "--debug-ir-output"     , SW_DEBUG_IR_OUTPUT    ),
            LONG_ONLY (      "--debug-irt-output"    , SW_DEBUG_IRT_OUTPUT   ),
            LONG_ONLY (      "--debug-all"           , SW_DEBUG_ALL          )
        };

        std::unordered_map <std::string, cli_setting_t> m_settings_map = {
            WSHORTHAND("-I", "--include-path"        , ST_INCLUDE_PATH       ),
            WSHORTHAND("-i", "--input"               , ST_INPUT              ),
            WSHORTHAND("-o", "--output"              , ST_OUTPUT             ),
            WSHORTHAND("-F", "--output-format"       , ST_OUTPUT_FORMAT      ),
            WSHORTHAND("-S", "--output-symbols"      , ST_OUTPUT_SYMBOLS     ),
            LONG_ONLY (      "--Xlat"                , ST_XLAT               ),
            LONG_ONLY (      "--help-target"         , ST_HELP_TARGET        )
        };

#undef WSHORTHAND
#undef LONG_ONLY

    public:
        void init(int argc, const char* argv[], error_logger_t* logger) {
            m_argc = argc;
            m_argv = argv;
            m_logger = logger;
        }

        bool get_switch(cli_switch_t sw) {
            return m_switches.contains(sw);
        }

        bool is_set(cli_setting_t st) {
            return m_settings.contains(st);
        }

        std::string get_setting(cli_setting_t st) {
            return m_settings[st];
        }

        bool parse() {
            if (m_argc == 1) {
                ERROR("No input files");

                return false;
            }

            for (int i = 1; i < m_argc; i++) {
                std::string arg(m_argv[i]);

                if (m_switches_map.contains(arg)) {
                    m_switches[m_switches_map[arg]] = true;

                    continue;
                }

                if (m_settings_map.contains(arg)) {
                    m_settings[m_settings_map[arg]] = std::string(m_argv[++i]);

                    continue;
                }

                if (m_settings.contains(ST_INPUT)) {
                    ERROR(fmt("Unknown setting \"%s\"", arg.c_str()));
                } else {
                    m_settings[ST_INPUT] = arg;
                }
            }

            return true;
        }
    };
}

#undef ERROR