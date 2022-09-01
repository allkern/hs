#pragma once

#include "../parser/parser.hpp"
#include "../error.hpp"
#include "instruction.hpp"

#include <vector>
#include <string>
#include <cstdint>

namespace hs {
    struct ir_generator_t {
        parser_output_t* m_po;
        error_logger_t* m_logger;

        std::vector <ir_instruction_t> m_binary;

        //std::unordered_map <std::string, int> m_global_map;
        std::unordered_map <std::string, int> m_local_map;

        int m_num_locals = 0, 
            m_num_args = 0;

    public:
        void init(parser_t* parser, error_logger_t* logger) {
            m_po = parser->get_output();

            m_logger = logger;
        }

        uint32_t generate_impl(expression_t* expr, int base, bool pointer = false, bool inside_fn = false) {
            switch (expr->get_type()) {
                case EX_FUNCTION_DEF: {
                    function_def_t* fd = (function_def_t*)expr;

                    //m_global_map.insert({fd->name, instructions});

                    for (function_arg_t& arg : fd->args) {
                        m_num_args++;

                        m_local_map.insert({arg.name, m_num_args * -4});
                    }

                    m_num_args++;

                    m_local_map.insert({"<return_address>", m_num_args * -4});

                    _log(debug, "----------------------");
                    _log(debug, "Function assembly:");

                    _log(debug, "LABEL %s", fd->name.c_str());

                    generate_impl(fd->body, base, false, true);

                    _log(debug, "MOV A0, R%u", base);

                    if (m_num_locals) {
                        _log(debug, "ADD SP %u", m_num_locals * 4);
                    }

                    _log(debug, "RET");

                    _log(debug, "----------------------");

                    m_num_locals = 0;
                    m_num_args = 0;

                    m_local_map.clear();

                    _log(debug, "MOV R%u, %s", base, fd->name.c_str());

                    return 1;
                } break;

                case EX_EXPRESSION_BLOCK: {
                    expression_block_t* eb = (expression_block_t*)expr;

                    if (!eb->block.size()) {
                        _log(debug, "NOP");

                        return 0;
                    }

                    for (int i = 0; i < eb->block.size(); i++) {
                        generate_impl(eb->block[i], base, pointer, true);
                    }

                    return 1;
                } break;

                case EX_VARIABLE_DEF: {
                    variable_def_t* vd = (variable_def_t*)expr;

                    _log(debug, "DEC SP");
                    _log(debug, "MOV R%u, SP", base);

                    if (inside_fn) {
                        m_num_locals++;

                        m_local_map.insert({vd->name, (m_num_locals + m_num_args) * -4});
                    }

                    return 1;
                } break;

                case EX_FUNCTION_CALL: {
                    function_call_t* fc = (function_call_t*)expr;

                    _log(debug, "MOV FP, SP");

                    for (expression_t* exp : fc->args) {
                        generate_impl(exp, base, false, inside_fn);

                        _log(debug, "PUSH R%u", base);
                    }

                    generate_impl(fc->addr, base, true, inside_fn);

                    _log(debug, "CALL R%u", base);
                    _log(debug, "MOV R%u, A0", base);
                    _log(debug, "MOV SP, FP");

                    return 1;
                } break;

                case EX_NUMERIC_LITERAL: {
                    numeric_literal_t* nl = (numeric_literal_t*)expr;

                    _log(debug, "MOV R%u, %u", base, nl->value);

                    return 1;
                } break;

                case EX_NAME_REF: {
                    name_ref_t* nr = (name_ref_t*)expr;

                    bool local = m_local_map.contains(nr->name);

                    if (local) {
                        if (!pointer) {
                            // If referring by value, then load the value from
                            // stack
                            _log(debug, "LOAD R%u, (%i)FP", base, m_local_map[nr->name]);
                        } else {
                            // Else, load the address in stack 
                            _log(debug, "LEA R%u, (%i)FP", base, m_local_map[nr->name]);
                        }
                    } else {
                        // If its a global variable, then load it's address
                        _log(debug, "MOV R%u, %s", base, nr->name);

                        // If referring by value, then load the value at that
                        // address                        
                        if (!pointer) {
                            _log(debug, "LOAD R%u R%u", base, base);
                        }
                    }

                    return 1;
                } break;

                case EX_BINARY_OP: {
                    binary_op_t* bo = (binary_op_t*)expr;

                    int lhs = generate_impl(bo->lhs, base, false, inside_fn);
                    int rhs = generate_impl(bo->rhs, base + lhs, false, inside_fn);

                    _log(debug, "ALU R%u, R%u", base, base + lhs);

                    return lhs + rhs;
                } break;

                case EX_ARRAY_ACCESS: {
                    array_access_t* aa = (array_access_t*)expr;

                    int addr = generate_impl(aa->addr, base, false, inside_fn);

                    _log(debug, "LOAD R%u, R%u", base, base);

                    return 1 + addr;
                } break;

                case EX_ASSIGNMENT: {
                    assignment_t* ae = (assignment_t*)expr;

                    int lhs = generate_impl(ae->assignee, base, true, inside_fn);
                    int rhs = generate_impl(ae->value, base + lhs, false, inside_fn);

                    _log(debug, "STORE R%u R%u", base, base + lhs);

                    return lhs + rhs;
                } break;
                
                default: return 1;
            }

            return 0;
        }

        void generate() {
            for (expression_t* expr : m_po->source) {
                _log(debug, "\n%s:", expr->print(0).c_str());
                generate_impl(expr, 0);
            }
        }
    };
}