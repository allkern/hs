#pragma once

#include "translator.hpp"

#include <sstream>
#include <unordered_map>

namespace hs {
    class ir_tr_x86_64_t : public ir_translator_t {
        std::vector <std::vector <ir_instruction_t>>* m_ir;
        
        error_logger_t* m_logger;

        std::unordered_map <std::string, std::string> m_register_map = {
            { "LR" , "<LR_unimplemented>" },
            { "TR" , "<TR_unimplemented>" },
            { "PC" , "%pc"  },
            { "SP" , "%rsp" },
            { "FP" , "%rbp" },
            { "R0" , "%rax" },
            { "R1" , "%rbx" },
            { "R2" , "%rcx" },
            { "R3" , "%rdx" },
            { "R4" , "%rsi" },
            { "R5" , "%rdi" },
            { "R6" , "%r8"  },
            { "R7" , "%r9"  },
            { "R8" , "%r10" },
            { "R9" , "%r11" },
            { "R10", "%r12" },
            { "R11", "%r13" },
            { "R12", "%r14" },
            { "R13", "%r15" },
            { "A0" , "%r15" }
        };

        enum bop_t {
            BP_ADD,
            BP_SUB,
            BP_MUL,
            BP_DIV,
            BP_AND,
            BP_OR,
            BP_XOR,
            BP_SHL,
            BP_SHR
        };

        std::unordered_map <std::string, bop_t> m_bop_map = {
            { "+" , BP_ADD },
            { "-" , BP_SUB },
            { "*" , BP_MUL },
            { "/" , BP_DIV },
            { "&" , BP_AND },
            { "|" , BP_OR  },
            { "^" , BP_XOR },
            { "<<", BP_SHL },
            { ">>", BP_SHR }
        };

        std::string map_binary_op(std::string bop_str) {
            bop_t bop = m_bop_map[bop_str];

            switch (bop) {
                case BP_ADD: return "add";
                case BP_SUB: return "sub";
                case BP_MUL: return "mul";
                case BP_DIV: return "div";
                case BP_AND: return "and";
                case BP_OR : return "or";
                case BP_XOR: return "xor";
                case BP_SHL: return "shl";
                case BP_SHR: return "shr";
            }

            return "unimplemented_operator";
        }

        static std::string map_branch(std::string cond) {
            if (cond == "EQ") return "je";
            if (cond == "NE") return "jne";
            if (cond == "AL") return "jmp";

            return "unimplemented_branch";
        }

        static std::string fmt_label(std::string label) {
            if (std::isdigit(label[0])) {
                return label;
            }

            std::ostringstream ss;

            for (char c : label) {
                if (c == '<') {
                    ss << '_';
                } else if (c == '>') {
                    continue;
                } else if (c == '.') {
                    ss << '_';
                } else if (c == '!') {
                    ss << '.';
                } else {
                    ss << c;
                }
            }

            return ss.str();
        }

    public:
        void init(ir_generator_t* irg, error_logger_t* logger) override {
            m_ir = irg->get_functions();
            m_logger = logger;
        }

        std::string translate() override {
            bool indented = false;

            std::ostringstream ss;

            //ss << ".text\n";

            for (std::vector <ir_instruction_t>& f : *m_ir) {
                for (ir_instruction_t& i : f) {
                    if (indented) ss << "    ";

                    switch (i.opcode) {
                        case IR_LABEL: {
                            ss << "\n" << fmt_label(i.args[0]) << ":";

                            indented = true;
                        } break;

                        case IR_ADDSP: {
                            ss << "add %esp, " << i.args[0];
                        } break;

                        case IR_ALU: {
                            std::string bop = map_binary_op(i.args[0]);

                            if ((bop == "shl") || (bop == "shr")) {
                                ss << "mov %rcx, " << m_register_map[i.args[2]] << "\n";
                                
                                if (indented) ss << "    ";

                                ss << bop << " %cl, " << m_register_map[i.args[1]]; 
                            } else {
                                ss << map_binary_op(i.args[0]) << " " << m_register_map[i.args[1]] << ", " << m_register_map[i.args[2]];
                            }
                        } break;

                        case IR_CALLR: {
                            ss << "call " << m_register_map[i.args[0]];
                        } break;

                        case IR_DECSP: {
                            ss << "sub %esp, 4";
                        } break;
                        
                        case IR_ADDFP: {
                            ss << "add %ebp, " << i.args[0];
                        } break;

                        case IR_LEAF: {
                            ss << "leal " << m_register_map[i.args[0]] << ", (%ebp-" << i.args[1] << ")";
                        } break;

                        case IR_LOADF: {
                            ss << "movl " << m_register_map[i.args[0]] << ", (%ebp-" << i.args[1] << ")";
                        } break;

                        case IR_LOADR: {
                            ss << "movl " << m_register_map[i.args[0]] << ", (" << m_register_map[i.args[1]] << ")";
                        } break;

                        case IR_MOV: {
                            ss << "mov " << m_register_map[i.args[0]] << ", " << m_register_map[i.args[1]]; 
                        } break;

                        case IR_MOVI: {
                            ss << "movabs " << m_register_map[i.args[0]] << ", " << fmt_label(i.args[1]);
                        } break;

                        case IR_NOP: {
                            ss << "nop";
                        } break;

                        case IR_PUSHR: {
                            ss << "push " << m_register_map[i.args[0]];
                        } break;

                        case IR_POPR: {
                            ss << "pop " << m_register_map[i.args[0]];
                        } break;

                        case IR_RET: {
                            ss << "ret";

                            indented = false;
                        } break;

                        case IR_PASSTHROUGH: {
                            ss << i.args[0];
                        } break;

                        case IR_STORE: {
                            ss << "movl (" << m_register_map[i.args[0]] << "), " << m_register_map[i.args[1]];
                        } break;

                        case IR_SUBSP: {
                            ss << "sub %esp, " << i.args[0];
                        } break;

                        case IR_BRANCH: {
                            ss << map_branch(i.args[0]) << " " << i.args[1];
                        } break;

                        case IR_DEFSTR: {
                            ss << ".asciiz \"" << i.args[0] << "\"";
                        } break;

                        case IR_DEFINE: {
                            ss << "#define " << i.args[0] << " " << i.args[1];
                        } break;
                        
                        case IR_UNDEF: {
                            ss << "#undef " << i.args[0];
                        } break;

                        case IR_DEFV: {
                            ss << ".dl " << fmt_label(i.args[1]);
                        } break;

                        case IR_CMPZB: {
                            ss << "cmp " << m_register_map[i.args[1]] << ", 0x0\n";
                            ss << map_branch(i.args[0]) << " " << i.args[2];
                        } break;

                        case IR_SECTION: {
                            ss << ".section " << i.args[0];
                        };

                        case IR_ORG: {
                            ss << ".org " << i.args[0];
                        } break;

                        case IR_ENTRY: {
                            ss << ".entry " << fmt_label(i.args[0]);
                        };
                    }

                    ss << std::endl;
                }
            }

            return ss.str();
        }
    };
}