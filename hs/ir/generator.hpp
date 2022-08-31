#pragma once

#include "../parser/parser.hpp"
#include "../error.hpp"

#include <vector>
#include <string>
#include <cstdint>

namespace hs {
    struct ir_generator_t {
        parser_output_t* m_po;
        error_logger_t* m_logger;

    public:
        void init(parser_t* parser, error_logger_t* logger) {
            m_po = parser->get_output();

            m_logger = logger;
        }

        uint32_t sp = 0x80000000;
        uint32_t fp = sp;

        int num_locals = 0;

        std::unordered_map <std::string, uint32_t> m_global_map;
        std::unordered_map <std::string, int     > m_local_map;

        int instructions = 0;

        uint32_t generate_impl(expression_t* expr, int base, bool pointer = false, bool inside_fn = false) {
            switch (expr->get_type()) {
                case EX_FUNCTION_DEF: {
                    function_def_t* fd = (function_def_t*)expr;

                    m_global_map.insert({fd->name, instructions});

                    int offset = -4;

                    for (function_arg_t& arg : fd->args) {
                        m_local_map.insert({arg.name, offset});

                        num_locals++;
                        offset -= 4;
                    }

                    m_local_map.insert({"<return_address>", offset - 4});
                    num_locals++;

                    _log(debug, "Function assembly:");

                    _log(debug, "LABEL %s", fd->name.c_str());

                    generate_impl(fd->body, base + 1, false, true);

                    _log(debug, "MOV A0, R%u", base + 1);
                    _log(debug, "RET");
                    instructions++;
                    instructions++;

                    _log(debug, "End of function assembly");

                    num_locals = 0;

                    m_local_map.clear();

                    _log(debug, "MOV R%u, fn_%s_addr", base, fd->name.c_str());
                    instructions++;

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
                    instructions++;
                    instructions++;

                    sp -= 4;

                    if (inside_fn) {
                        num_locals++;
                        m_local_map.insert({vd->name, num_locals * -4});
                    } else {
                        m_global_map.insert({vd->name, sp});
                    }

                    return 1;
                } break;

                case EX_FUNCTION_CALL: {
                    function_call_t* fc = (function_call_t*)expr;

                    _log(debug, "MOV FP, SP");
                    instructions++;

                    for (expression_t* exp : fc->args) {
                        generate_impl(exp, base, false, inside_fn);

                        _log(debug, "PUSH R%u", base);
                        instructions++;
                    }

                    generate_impl(fc->addr, base, true, inside_fn);

                    _log(debug, "CALL R%u", base); instructions++;
                    _log(debug, "MOV R%u, A0", base); instructions++;
                    _log(debug, "MOV SP, FP"); instructions++;

                    return 1;
                } break;

                case EX_NUMERIC_LITERAL: {
                    numeric_literal_t* nl = (numeric_literal_t*)expr;

                    _log(debug, "MOV R%u, %u", base, nl->value);
                    instructions++;

                    return 1;
                } break;

                case EX_NAME_REF: {
                    name_ref_t* nr = (name_ref_t*)expr;

                    bool global = m_global_map.contains(nr->name);

                    if (global) {
                        _log(debug, "MOV R%u, %08x", base, m_global_map[nr->name]);
                        instructions++;
                        
                        if (!pointer) {
                            _log(debug, "LOAD R%u R%u", base, base);
                            instructions++;
                        }
                    } else {
                        if (!pointer) {
                            _log(debug, "LOAD R%u, (%i)FP", base, m_local_map[nr->name]);
                            instructions++;
                        }
                    }

                    return 1;
                } break;

                case EX_BINARY_OP: {
                    binary_op_t* bo = (binary_op_t*)expr;

                    int lhs = generate_impl(bo->lhs, base, false, inside_fn);
                    int rhs = generate_impl(bo->rhs, base + lhs, false, inside_fn);

                    _log(debug, "ALU R%u, R%u", base, base + lhs);
                    instructions++;

                    return lhs + rhs;
                } break;

                case EX_ARRAY_ACCESS: {
                    array_access_t* aa = (array_access_t*)expr;

                    int addr = generate_impl(aa->addr, base, false, inside_fn);

                    _log(debug, "LOAD R%u, R%u", base, base);

                    instructions++;

                    return 1 + addr;
                } break;

                case EX_ASSIGNMENT: {
                    assignment_t* ae = (assignment_t*)expr;

                    int lhs = generate_impl(ae->assignee, base, true, inside_fn);
                    int rhs = generate_impl(ae->value, base + lhs, false, inside_fn);

                    _log(debug, "STORE R%u R%u", base, base + lhs);
                    instructions++;

                    return lhs + rhs;
                } break;
                
                default: return 1;
            }

            return 0;
        }

        void generate() {
            for (expression_t* expr : m_po->source) {
                _log(debug, "%s:", expr->print(0).c_str());
                generate_impl(expr, 0);
            }
        }
    };
}