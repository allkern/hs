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
        
        int m_loops = 0;

        std::unordered_map <std::string, int> m_local_map;

        int m_num_locals = 0, 
            m_num_args = 0;
        
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
                case EX_WHILE_LOOP: {
                    while_loop_t* wl = (while_loop_t*)expr;

                    int m_this_loop = m_loops++;

                    // To-do: clean this up
                    append({IR_LABEL, "!L" + std::to_string(m_this_loop)});

                    generate_impl(wl->body, base, false, inside_fn);

                    generate_impl(wl->condition, base, false, inside_fn);

                    append({IR_BRANCH, "NE", "L" + std::to_string(m_this_loop)});
                } break;

                case EX_FUNCTION_DEF: {
                    function_def_t* fd = (function_def_t*)expr;

                    begin_function();

                    for (function_arg_t& arg : fd->args) {
                        m_num_args++;

                        m_local_map.insert({arg.name, m_num_args * 4});
                    }

                    m_num_args++;

                    m_local_map.insert({"<return_address>", m_num_args * 4});

                    append({IR_LABEL, fd->name});

                    generate_impl(fd->body, base, false, true);

                    append({IR_MOV, "A0", "R" + std::to_string(base)});

                    if (m_num_locals) {
                        append({IR_ADDSP, std::to_string(m_num_locals * 4)});
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

                    append({IR_MOV, "FP", "SP"});

                    for (expression_t* exp : fc->args) {
                        generate_impl(exp, base, false, inside_fn);

                        append({IR_PUSHR, "R" + std::to_string(base)});
                    }

                    generate_impl(fc->addr, base, true, inside_fn);

                    append({IR_CALLR, "R" + std::to_string(base)});
                    append({IR_MOV, "R" + std::to_string(base), "A0"});
                    append({IR_MOV, "SP", "FP"});

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

                    int lhs = generate_impl(bo->lhs, base, false, inside_fn);
                    int rhs = generate_impl(bo->rhs, base + lhs, false, inside_fn);

                    append({IR_ALU, std::string(1, bo->bop), "R" + std::to_string(base), "R" + std::to_string(base + lhs)});

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

                    int lhs = generate_impl(ae->assignee, base, true, inside_fn);
                    int rhs = generate_impl(ae->value, base + lhs, false, inside_fn);

                    append({IR_STORE, "R" + std::to_string(base), "R" + std::to_string(base + lhs)});

                    return lhs + rhs;
                } break;
                
                default: return 1;
            }

            return 0;
        }

        void generate() {
            m_functions.at(0).push_back({IR_LABEL, "<ENTRY>"});

            for (expression_t* expr : m_po->source) {
                //_log(debug, "\n%s:", expr->print(0).c_str());
                generate_impl(expr, 0);
            }
            
            // Function call semantics
            append({IR_MOV, "FP", "SP"});
            m_functions.at(0).push_back({IR_MOVI, "R0", "<global>.main"});
            m_functions.at(0).push_back({IR_CALLR, "R0"});
            append({IR_MOV, "R0", "A0"});
            append({IR_MOV, "SP", "FP"});

            // This will only work with Hyrisc for now.
            // just ignore and remove whenever needed
            m_functions.at(0).push_back({IR_PASSTHROUGH, "debug"});
        }
    };
}