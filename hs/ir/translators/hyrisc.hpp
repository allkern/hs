#pragma once

#include "translator.hpp"

#include <sstream>

namespace hs {
    class ir_tr_hyrisc_t : public ir_translator_t {
        std::vector <std::vector <ir_instruction_t>>* m_ir;
        
        error_logger_t* m_logger;

        static std::string map_register(std::string reg) {
            if (reg == "PC") return "pc";
            if (reg == "SP") return "sp";
            if (reg == "LR") return "lr";
            if (reg == "FP") return "fp";
            if (reg == "TR") return "tr";

            if (reg[0] == 'A') {
                return "a" + std::string(1, reg[1]);
            }

            if (reg[0] == 'R') {
                int number = std::stoi(reg.substr(1)) + 1;

                return "r" + std::to_string(number);
            }

            return "r0";
        }

        static std::string map_binary_op(char bop) {
            switch (bop) {
                case '+': return "add.u";
                case '-': return "sub.u";
                case '*': return "mul.u";
                case '/': return "div.u";
                case '&': return "and";
                case '|': return "or";
                case '^': return "xor";
            }

            return "unimplemented_operator";
        }

        static std::string map_branch(std::string cond) {
            if (cond == "EQ") return "beq";
            if (cond == "NE") return "bne";
            if (cond == "AL") return "bra";

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

            ss << ".text\n";

            for (std::vector <ir_instruction_t>& f : *m_ir) {
                for (ir_instruction_t& i : f) {
                    if (indented) ss << "    ";

                    switch (i.opcode) {
                        case IR_LABEL: {
                            ss << "\n" << fmt_label(i.args[0]) << ":";

                            indented = true;
                        } break;

                        case IR_ADDSP: {
                            ss << "add.u sp, " << i.args[0];
                        } break;

                        case IR_ALU: {
                            ss << map_binary_op(i.args[0][0]) << " " << map_register(i.args[1]) << ", " << map_register(i.args[2]);
                        } break;

                        case IR_CALLR: {
                            ss << "call " << map_register(i.args[0]);
                        } break;

                        case IR_DECSP: {
                            ss << "dec.l sp";
                        } break;
                        
                        case IR_ADDFP: {
                            ss << "add.u fp, " << i.args[0];
                        } break;

                        case IR_LEAF: {
                            ss << "lea.l " << map_register(i.args[0]) << ", [fp-" << i.args[1] << "]";
                        } break;

                        case IR_LOADF: {
                            ss << "load.l " << map_register(i.args[0]) << ", [fp-" << i.args[1] << "]";
                        } break;

                        case IR_LOADR: {
                            ss << "load.l " << map_register(i.args[0]) << ", [" << map_register(i.args[1]) << "]";
                        } break;

                        case IR_MOV: {
                            ss << "mov " << map_register(i.args[0]) << ", " << map_register(i.args[1]); 
                        } break;

                        case IR_MOVI: {
                            ss << ".load32 " << map_register(i.args[0]) << " " << fmt_label(i.args[1]);
                        } break;

                        case IR_NOP: {
                            ss << "nop";
                        } break;

                        case IR_PUSHR: {
                            ss << "push " << map_register(i.args[0]);
                        } break;

                        case IR_POPR: {
                            ss << "pop " << map_register(i.args[0]);
                        } break;

                        case IR_RET: {
                            ss << "ret";

                            indented = false;
                        } break;

                        case IR_PASSTHROUGH: {
                            ss << i.args[0];
                        } break;

                        case IR_STORE: {
                            ss << "store.l [" << map_register(i.args[0]) << "], " << map_register(i.args[1]);
                        } break;

                        case IR_SUBSP: {
                            ss << "sub.u sp, " << i.args[0];
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

                        case IR_CMPRI: {
                            ss << "cmp " << map_register(i.args[0]) << ", " << i.args[1];
                        } break;
                    }

                    ss << std::endl;
                }
            }

            return ss.str();
        }
    };
}