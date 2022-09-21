#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <stack>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/context.hpp"
#include "preprocessor/preprocessor.hpp"
#include "ir/generator.hpp"
#include "ir/instruction.hpp"
#include "ir/translators/translator.hpp"
#include "assembler/assembler.hpp"
#include "cli.hpp"

// IR Translators
#include "ir/translators/hyrisc.hpp"

// Assemblers
#include "assembler/hyrisc/assembler.hpp"

#ifndef OS_VERSION
#define OS_VERSION "unknown"
#endif
#ifndef BUILD_INFO
#define BUILD_INFO
#endif
#ifndef HS_VERSION
#define HS_VERSION "latest"
#endif
#ifndef HS_COMMIT_HASH
#define HS_COMMIT_HASH
#endif

#define STR1(m) #m
#define STR(m) STR1(m)

namespace hs {
    static std::string m_version_text =
#ifdef _WIN32
        "hs.exe (" STR(OS_VERSION) STR(BUILD_INFO) ") " STR(HS_VERSION) "-" STR(HS_COMMIT_HASH) "\n"
#elif __linux__
        "hs (" STR(OS_VERSION) STR(BUILD_INFO) ") " STR(HS_VERSION) "-" STR(HS_COMMIT_HASH) "\n"
#else
        "hs (" STR(OS_VERSION) STR(BUILD_INFO) ") " STR(HS_VERSION) "-" STR(HS_COMMIT_HASH) "\n"
#endif
        "Copyright (C) 2022 Allkern/Lycoder (Lisandro Alarcon)\n"
        "This is free software; see the source for copying conditions.  There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
    
    static std::string m_help_text =
        "Usage: hs [options] file\n"
        "Options:\n"
        "  -i, --input <file>        Specify an input file\n"
        "  -o, --output <file>       Specify an output file (extension doesn't specify\n"
        "                            output format)\n"
        "  -I, --include-paths <path,path,...>\n"
        "                            Specify additional comma-separated include search\n"
        "                            paths\n"
        "      --system-include <path>\n"
        "                            Specify system include search path\n"
        "  -F, --output-format <fmt> Specify output format (raw, elf32, etc.)\n"
        "      --help-target <tgt>   Get target-specific help\n"
        "      --Xlat <opt,opt,...>  Pass comma-separated options to the IR translator\n"
        "  -T, --target-arch <arch>  Specify a target architecture (default Hyrisc)\n"
        "  -v, --version             Display compiler version information\n"
        "  -q, --quiet               Display minimal/no information output\n"
        "  -V, --verbose             Display maximal/all information output\n"
        "  -a, --assemble            Assemble input using the target's assembler\n"
        "  -L, --log                 Log compiler information output to a file (a.log)\n"
        "  -A, --output-assembly     Output source's assembly to a file (a.s)\n"
        "  -S, --output-symbols      Output source's symbols to a file (a.sym)\n"
        "      --only-symbols        Only output a.sym\n"
        "      --help                Display this information\n"
        "      --debug-lexer-output  Display lexer debugging information\n"
        "      --debug-parser-output Display parser debugging information\n"
        "      --debug-ir-output     Display IR generator debugging information\n"
        "      --debug-irt-output    Display target's IR translator debugging\n"
        "                            information\n"
        "      --debug-all           Display debugging information from all stages\n"
        "      --stdin               Get input stream from stdin\n"
        "      --stdout              Compile output to stdout\n"
        "      --stdio               Get input stream from stdin and compile output to\n"
        "                            stdout\n"
        "\n"
        "Options need to be specified individually (i.e. no \"-VvqaL...\") and\n"
        "arguments to options need to be passed leaving a space between the option\n"
        "and argument. e.g. -T x86_64. Comma-separated arguments must be written\n"
        "without spaces between each argument. e.g. -I inc,inc2,inc3\n"
        "\n"
        "For bug reporting please file an issue on:\n"
        "https://github.com/lycoder/hs/issues";
    
    enum target_arch_t {
        TGT_ARCH_HYRISC,
        TGT_ARCH_X86,
        TGT_ARCH_X64,
        TGT_ARCH_ARM,
        TGT_ARCH_ARM_THUMB,
        TGT_ARCH_AARCH64,
        TGT_ARCH_MIPS,
        TGT_ARCH_RV32,
        TGT_ARCH_RV64
    };

    std::unordered_map <std::string, target_arch_t> m_target_arch_map = {
          { "hyrisc" , TGT_ARCH_HYRISC    },
          { "x86"    , TGT_ARCH_X86       },
          { "x86-32" , TGT_ARCH_X86       },
          { "x86_32" , TGT_ARCH_X86       },
          { "x64"    , TGT_ARCH_X64       },
          { "x86-64" , TGT_ARCH_X64       },
          { "x86_64" , TGT_ARCH_X64       },
          { "arm"    , TGT_ARCH_ARM       },
          { "thumb"  , TGT_ARCH_ARM_THUMB },
          { "aarch64", TGT_ARCH_AARCH64   },
          { "mips"   , TGT_ARCH_MIPS      },
          { "riscv"  , TGT_ARCH_RV32      },
          { "riscv32", TGT_ARCH_RV32      },
          { "riscv64", TGT_ARCH_RV64      },
          { "rv"     , TGT_ARCH_RV32      },
          { "rv32"   , TGT_ARCH_RV32      },
          { "rv64"   , TGT_ARCH_RV64      }
    };

    class compiler_t {
        error_logger_t              m_logger;
        cli_parser_t                m_cli;
        preprocessor_t              m_hspp;
        preprocessor_t              m_aspp;
        lexer_t                     m_lexer;
        parser_t                    m_parser;
        contextualizer_t            m_context;
        ir_generator_t              m_irg;
        ir_translator_t*            m_translator;
        assembler_t*                m_assembler;
        std::ostream*               m_output = nullptr;
        std::istream*               m_input = nullptr;
        std::ifstream               m_input_file;
        std::ofstream               m_output_file;
        std::vector <std::string>   m_include_paths = { "." };
        std::string                 m_system_include;
        std::string                 m_filename;

        void parse_csv(std::string str, std::vector <std::string>& dest) {
            std::string value;

            for (char c : str) {
                if (c == ',') {
                    dest.push_back(value);

                    value.clear();
                } else {
                    // Remove double quotes
                    if (c != '\"') 
                        value.push_back(c);
                }
            }
        }

        void load_target_specific_code(target_arch_t tgt) {
            switch (tgt) {
                case TGT_ARCH_HYRISC: {
                    m_translator = new ir_tr_hyrisc_t;
                    m_assembler = new assembler_hyrisc_t;
                } break;

                // We don't support other architectures yet
                default: {
                    m_translator = nullptr;
                    m_assembler = nullptr;
                }
            }
        }
    
    public:
        bool init(int argc, const char** argv) {
            m_cli.init(argc, argv, &m_logger);

            if (!m_cli.parse()) {
                m_logger.print_error("hs", "compilation terminated", 0, 0, 0, false, true);

                return false;
            }

            if (m_cli.get_switch(SW_VERSION)) {
                std::cout << m_version_text << std::endl;

                return false;
            }

            if (m_cli.get_switch(SW_HELP)) {
                std::cout << m_help_text << std::endl;

                return false;
            }

            if (m_cli.is_set(ST_HELP_TARGET)) {
                std::cout << m_cli.get_setting(ST_HELP_TARGET) << "-specific help unimplemented :(\n" << std::endl;

                return false;
            }

            if (m_cli.is_set(ST_TARGET_ARCH)) {
                std::string tgt = m_cli.get_setting(ST_TARGET_ARCH);

                if (!m_target_arch_map.contains(tgt)) {
                    m_logger.print_error(
                        "hs",
                        fmt("Target architecture \"%s\" not supported", tgt.c_str()),
                        0, 0, 0, false, true
                    );

                    return false;
                }

                load_target_specific_code(m_target_arch_map[m_cli.get_setting(ST_TARGET_ARCH)]);
            } else {
                load_target_specific_code(TGT_ARCH_HYRISC);
            }

            if (!m_translator) {
                m_logger.print_error(
                    "hs",
                    fmt("Target architecture \"%s\" IR translator unimplemented", m_cli.get_setting(ST_TARGET_ARCH).c_str())
                    , 0, 0, 0, false, true
                );

                return false;
            }

            if (!m_assembler) {
                m_logger.print_error(
                    "hs",
                    fmt("Target architecture \"%s\" assembler unimplemented", m_cli.get_setting(ST_TARGET_ARCH).c_str())
                    , 0, 0, 0, false, true
                );

                return false;
            }

            if (m_cli.is_set(ST_INCLUDE_PATHS)) {
                parse_csv(m_cli.get_setting(ST_INCLUDE_PATHS), m_include_paths);
            }

            if (m_cli.is_set(ST_SYSTEM_INCLUDE)) {
                m_system_include = m_cli.get_setting(ST_SYSTEM_INCLUDE);
            } else {
#ifdef _WIN32
                m_system_include = "";
#elif __linux__
                m_system_include = "/usr/include/hs";
#else
                m_system_include = "";
#endif
            }

            if (m_cli.get_switch(SW_STDIN) || m_cli.get_switch(SW_STDIO)) m_input = &std::cin;
            if (m_cli.get_switch(SW_STDOUT) || m_cli.get_switch(SW_STDIO)) m_output = &std::cout;

            if (m_cli.is_set(ST_INPUT) && !(m_cli.get_switch(SW_STDIN) || m_cli.get_switch(SW_STDIO))) {
                m_filename = m_cli.get_setting(ST_INPUT);

                m_input_file.open(m_filename, std::ios::binary);

                if (!(m_input_file.is_open() && m_input_file.good())) {
                    m_logger.print_error("hs", fmt("Couldn't open input file \"%s\"", m_cli.get_setting(ST_INPUT).c_str()), 0, 0, 0, false, true);
                    m_logger.print_error("hs", "compilation terminated", 0, 0, 0, false, true);

                    return false;
                }

                m_input = &m_input_file;
            }

            if (!m_input) {
                m_logger.print_error("hs", "No input files", 0, 0, 0, false, true);
                m_logger.print_error("hs", "compilation terminated", 0, 0, 0, false, true);

                return false;
            }

            if (m_cli.is_set(ST_OUTPUT) && !(m_cli.get_switch(SW_STDOUT) || m_cli.get_switch(SW_STDIO))) {
                m_output_file.open(m_cli.get_setting(ST_OUTPUT), std::ios::binary);

                if (!(m_output_file.is_open() && m_output_file.good())) {
                    m_logger.print_error("hs", fmt("Couldn't open output file \"%s\"", m_cli.get_setting(ST_OUTPUT).c_str()), 0, 0, 0, false, true);
                    m_logger.print_error("hs", "compilation terminated", 0, 0, 0, false, true);

                    return false;
                }
            } else {
                m_output_file.open("a.out", std::ios::binary);
            }

            m_output = &m_output_file;

            return true;
        }

        bool compile() {
            m_logger.init(m_input, m_filename);

            if (m_cli.get_switch(SW_ASSEMBLE)) {
                m_aspp.init(m_input, &m_include_paths, m_system_include, &m_logger);
                m_aspp.preprocess();

                m_assembler->init(m_aspp.get_output(), m_output, &m_logger);
                m_assembler->assemble();

                return true;
            }
            
            m_hspp.init(m_input, &m_include_paths, m_system_include, &m_logger);
            m_hspp.preprocess();

            m_lexer.init(m_hspp.get_output(), &m_logger);
            m_lexer.lex();

            if (m_cli.get_switch(SW_DEBUG_LEXER_OUTPUT) || m_cli.get_switch(SW_DEBUG_ALL)) {
                _log(debug, "Lexer output:");

                for (hs::lexer_token_t token : *m_lexer.get_output()) {
                    std::cout << "(" << token.line + 1 << ", " << token.offset + 1 << "): "
                              << "type: " << hs::lexer_token_type_names[token.type] << ", "
                              << "text: " << token.text << ")\n";
                }
            }

            m_parser.init(m_lexer.get_output(), &m_logger);
            m_parser.parse();

            m_context.init(&m_parser, &m_logger);
            m_context.contextualize();

            if (m_cli.get_switch(SW_DEBUG_PARSER_OUTPUT) || m_cli.get_switch(SW_DEBUG_ALL)) {
                _log(debug, "Contextualized parser output:");

                for (expression_t* expr : m_parser.get_output()->source) {
                    _log(debug, "expression:\n%s", expr->print(0).c_str());
                }
            }

            m_irg.init(&m_parser, &m_logger);
            m_irg.generate();

            if (m_cli.get_switch(SW_DEBUG_IR_OUTPUT) || m_cli.get_switch(SW_DEBUG_ALL)) {
                _log(debug, "\nIR Generator output:");
                // To-do
                // std::vector <std::vector <ir_instruction_t>>* funcs = m_irg.get_functions();

                // for (std::vector <ir_instruction_t>* fn : *funcs) {

                // }
            }

            m_translator->init(&m_irg, &m_logger);
            std::string assembly = m_translator->translate();

            if (m_cli.get_switch(SW_DEBUG_IRT_OUTPUT) || m_cli.get_switch(SW_DEBUG_ALL)) {
                _log(debug, "\nIR Translator output:");

                std::cout << assembly;
            }

            std::stringstream assembly_stream(assembly);

            m_aspp.init(&assembly_stream, &m_include_paths, m_system_include, &m_logger);
            m_aspp.preprocess();

            m_assembler->init(m_aspp.get_output(), m_output, &m_logger);
            m_assembler->assemble();

            if (m_cli.get_switch(SW_PRINT_SUCCESS)) {
                _log(ok, "Done compiling");
            }

            return true;
        }
    };
}