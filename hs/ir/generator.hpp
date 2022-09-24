#pragma once

#include "../parser/parser.hpp"
#include "../error.hpp"
#include "instruction.hpp"

#include <stack>
#include <vector>
#include <string>
#include <cstdint>

namespace hs {
    class ir_generator_t {
        parser_output_t* m_po;
        error_logger_t* m_logger;

        std::vector <ir_instruction_t> m_dummy;

        std::vector <std::vector <ir_instruction_t>> m_functions;

        int m_current = 0,
            m_prev = m_current;
        
        int m_loops = 0,
            m_strings = 0;

        struct string_t {
            std::string name, value;
        };
        
        std::vector <string_t> m_pending_strings;

        std::unordered_map <std::string, int> m_local_map;

        int m_num_locals = 0, 
            m_num_args = 0;

        std::string get_variable_name(std::string str) {
            return "arg_" + str.substr(str.find_last_of('.') + 1);
        }

        std::string new_string(std::string str) {
            string_t string;

            string.name = "S" + std::to_string(m_strings++);
            string.value = str;

            m_pending_strings.push_back(string);

            return string.name; 
        }
        
        void begin_function() {
            m_prev = m_current;
            m_current = m_functions.size();
            m_loops = 0;

            m_functions.push_back(m_dummy);
        }

        void append(ir_instruction_t ins) {
            m_functions.at(m_current).push_back(ins);
        }

        void end_function() {
            m_current = m_prev;
        }

    public:
        std::vector <std::vector <ir_instruction_t>>* get_functions() {
            return &m_functions;
        }

        void init(parser_t* parser, error_logger_t* logger) {
            m_po = parser->get_output();

            m_logger = logger;

            m_functions.resize(1);
        }

        uint32_t generate_impl(expression_t* expr, int base, bool pointer = false, bool inside_fn = false) {
            switch (expr->get_type()) {
                case EX_IF_ELSE: {
                    if_else_t* ie = (if_else_t*)expr;

                    int m_this_loop = m_loops++;

                    generate_impl(ie->cond, base, false, inside_fn);

                    append({IR_CMPRI, "R" + std::to_string(base), "0"});

                    append({IR_BRANCH, "EQ", (ie->else_expr ? "E" : "L") + std::to_string(m_this_loop)});

                    generate_impl(ie->if_expr, base, false, inside_fn);

                    if (ie->else_expr) {
                        append({IR_BRANCH, "AL", "L" + std::to_string(m_this_loop)});

                        append({IR_LABEL, "!E" + std::to_string(m_this_loop)});

                        generate_impl(ie->else_expr, base, false, inside_fn);
                    }

                    append({IR_LABEL, "!L" + std::to_string(m_this_loop)});
                } break;
                
                case EX_STRING_LITERAL: {
                    string_literal_t* sl = (string_literal_t*)expr;

                    std::string label = new_string(sl->str);

                    append({IR_MOVI, "R" + std::to_string(base), label});

                    return 1;
                } break;

                case EX_WHILE_LOOP: {
                    while_loop_t* wl = (while_loop_t*)expr;

                    int m_this_loop = m_loops++;

                    // To-do: clean this up
                    append({IR_LABEL, "!L" + std::to_string(m_this_loop)});

                    generate_impl(wl->condition, base, false, inside_fn);

                    append({IR_CMPRI, "R" + std::to_string(base), "0"});

                    append({IR_BRANCH, "EQ", "E" + std::to_string(m_this_loop)});

                    generate_impl(wl->body, base, false, inside_fn);

                    append({IR_BRANCH, "AL", "L" + std::to_string(m_this_loop)});

                    append({IR_LABEL, "!E" + std::to_string(m_this_loop)});
                } break;

                case EX_FUNCTION_DEF: {
                    function_def_t* fd = (function_def_t*)expr;

                    begin_function();

                    append({IR_LABEL, fd->name});

                    int arg_frame_pos = 1;

                    for (function_arg_t& arg : fd->args) {
                        m_num_args++;

                        append({IR_DEFINE, get_variable_name(arg.name), "[fp-" + std::to_string(4 * (arg_frame_pos++)) + "]"});

                        m_local_map.insert({arg.name, m_num_args * 4});
                    }

                    m_num_args++;

                    m_local_map.insert({"<return_address>", m_num_args * 4});

                    generate_impl(fd->body, base, false, true);

                    append({IR_MOV, "A0", "R" + std::to_string(base)});

                    if (m_num_locals) {
                        append({IR_ADDSP, std::to_string(m_num_locals * 4)});
                    }

                    for (function_arg_t& arg : fd->args) {
                        append({IR_UNDEF, get_variable_name(arg.name)});
                    }

                    append({IR_RET});

                    end_function();

                    m_num_locals = 0;
                    m_num_args = 0;

                    m_local_map.clear();

                    append({IR_MOVI, "R" + std::to_string(base), fd->name});

                    return 1;
                } break;

                case EX_EXPRESSION_BLOCK: {
                    expression_block_t* eb = (expression_block_t*)expr;

                    if (!eb->block.size()) {
                        append({IR_NOP});

                        return 0;
                    }

                    for (int i = 0; i < eb->block.size(); i++) {
                        generate_impl(eb->block[i], base, pointer, true);
                    }

                    return 1;
                } break;

                case EX_ASM_BLOCK: {
                    asm_block_t* ab = (asm_block_t*)expr;

                    append({IR_PASSTHROUGH, ab->assembly});

                    // Possible improvement, account for registers
                    // used within asm block
                    return 0;
                } break;

                case EX_VARIABLE_DEF: {
                    variable_def_t* vd = (variable_def_t*)expr;

                    append({IR_DECSP});
                    append({IR_MOV, "R" + std::to_string(base), "SP"});

                    if (inside_fn) {
                        m_num_locals++;

                        m_local_map.insert({vd->name, (m_num_locals + m_num_args) * 4});
                    }

                    return 1;
                } break;

                case EX_FUNCTION_CALL: {
                    function_call_t* fc = (function_call_t*)expr;

                    append({IR_PUSHR, "FP"});

                    for (expression_t* exp : fc->args) {
                        generate_impl(exp, base, false, inside_fn);

                        append({IR_PUSHR, "R" + std::to_string(base)});
                    }

                    append({IR_MOV, "FP", "SP"});
                    append({IR_ADDFP, std::to_string(fc->args.size() * 4)});

                    generate_impl(fc->addr, base, true, inside_fn);

                    append({IR_CALLR, "R" + std::to_string(base)});
                    append({IR_MOV, "R" + std::to_string(base), "A0"});

                    append({IR_MOV, "SP", "FP"});
                    append({IR_POPR, "FP"});

                    return 1;
                } break;

                case EX_NUMERIC_LITERAL: {
                    numeric_literal_t* nl = (numeric_literal_t*)expr;

                    append({IR_MOVI, "R" + std::to_string(base), std::to_string(nl->value)});

                    return 1;
                } break;

                case EX_NAME_REF: {
                    name_ref_t* nr = (name_ref_t*)expr;

                    bool local = m_local_map.contains(nr->name);

                    if (local) {
                        if (!pointer) {
                            // If referring by value, then load the value from
                            // stack

                            append({IR_LOADF, "R" + std::to_string(base), std::to_string(m_local_map[nr->name])});
                        } else {
                            // Else, load the address in stack 
                            append({IR_LEAF, "R" + std::to_string(base), std::to_string(m_local_map[nr->name])});
                        }
                    } else {
                        // If its a global variable, then load it's address
                        append({IR_MOVI, "R" + std::to_string(base), nr->name});

                        // If referring by value, then load the value at that
                        // address                        
                        if (!pointer) {
                            append({IR_LOADR, "R" + std::to_string(base), "R" + std::to_string(base)});
                        }
                    }

                    return 1;
                } break;

                case EX_BINARY_OP: {
                    binary_op_t* bo = (binary_op_t*)expr;

                    int rhs = generate_impl(bo->rhs, base, false, inside_fn);
                    int lhs = generate_impl(bo->lhs, base + rhs, false, inside_fn);
                    
                    // To-do: Check this
                    append({IR_ALU, bo->bop, "R" + std::to_string(base + rhs), "R" + std::to_string(base)});
                    append({IR_MOV, "R" + std::to_string(base), "R" + std::to_string(base + rhs)});

                    return lhs + rhs;
                } break;

                case EX_ARRAY_ACCESS: {
                    array_access_t* aa = (array_access_t*)expr;

                    int addr = generate_impl(aa->addr, base, false, inside_fn);

                    if (!pointer) { 
                        append({IR_LOADR, "R" + std::to_string(base), "R" + std::to_string(base)});
                    }

                    return addr;
                } break;

                case EX_ASSIGNMENT: {
                    assignment_t* ae = (assignment_t*)expr;

                    int rhs = generate_impl(ae->value, base, false, inside_fn);
                    int lhs = generate_impl(ae->assignee, base + rhs, true, inside_fn);

                    append({IR_STORE, "R" + std::to_string(base + rhs), "R" + std::to_string(base)});

                    return lhs + rhs;
                } break;
                
                default: return 1;
            }

            return 0;
        }

        void generate() {
            m_functions.at(0).push_back({IR_LABEL, "<ENTRY>"});

            for (expression_t* expr : m_po->source) {
                generate_impl(expr, 0);
            }
            
            // Function call semantics
            m_functions.at(0).push_back({IR_PUSHR, "FP"});
            m_functions.at(0).push_back({IR_MOV, "FP", "SP"});
            m_functions.at(0).push_back({IR_MOVI, "R0", "<global>.main"});
            m_functions.at(0).push_back({IR_CALLR, "R0"});
            m_functions.at(0).push_back({IR_MOV, "R0", "A0"});
            m_functions.at(0).push_back({IR_MOV, "SP", "FP"});
            m_functions.at(0).push_back({IR_POPR, "FP"});

            // This will only work with Hyrisc for now.
            // just ignore and remove whenever needed
            m_functions.at(0).push_back({IR_PASSTHROUGH, "debug"});

            for (string_t& str : m_pending_strings) {
                m_functions.at(m_functions.size() - 1).push_back({IR_LABEL, str.name});
                m_functions.at(m_functions.size() - 1).push_back({IR_DEFSTR, str.value});
            }
        }
    };
}