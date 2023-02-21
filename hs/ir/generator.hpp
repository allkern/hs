#pragma once

#include "../parser/expressions/type.hpp"
#include "../parser/parser.hpp"
#include "../error.hpp"
#include "../cli.hpp"

#include "instruction.hpp"

#include <stack>
#include <vector>
#include <string>
#include <cstdint>

namespace hs {
    class ir_generator_t {
        parser_output_t* m_po;
        error_logger_t* m_logger;
        cli_parser_t* m_cli;

        std::vector <ir_instruction_t> m_dummy;

        std::vector <std::vector <ir_instruction_t>> m_functions;

        int m_current_function = 0;
        
        std::stack <int> m_current_loops;

        int m_strings = 0,
            m_arrays = 0,
            m_blobs = 0;

        struct string_t {
            std::string name, value;
        };

        struct blob_def_t {
            std::string name, file;
        };
        
        std::vector <string_t> m_pending_strings;
        std::vector <array_t> m_pending_arrays;
        std::vector <blob_def_t> m_pending_blobs;


        struct variable_t {
            int address;

            std::string type;
        };

        std::unordered_map <std::string, variable_t> m_dummy_local_map;

        std::stack <std::unordered_map <std::string, variable_t>> m_local_maps;

        std::stack <int> m_current_num_locals;
        std::stack <int> m_current_num_args;

        std::string get_variable_name(std::string str) {
            return "arg_" + str.substr(str.find_last_of('.') + 1);
        }

        std::string new_string(std::string str) {
            string_t string;

            string.name = "DS" + std::to_string(m_strings++);
            string.value = str;

            m_pending_strings.push_back(string);

            return string.name; 
        }

        std::string new_array(array_t arr) {
            m_pending_arrays.push_back(arr);

            return "DA" + std::to_string(m_arrays++);
        }
        
        std::string new_blob(std::string file) {
            blob_def_t blob_def;

            blob_def.file = file;
            blob_def.name = "DB" + std::to_string(m_blobs++);

            m_pending_blobs.push_back(blob_def);

            return blob_def.name;
        }
        
        void begin_function() {
            m_current_loops.push(0);

            m_current_function++;
            m_functions.push_back(m_dummy);
        }

        void append(ir_instruction_t ins) {
            m_functions.at(m_current_function).push_back(ins);
        }

        void end_function() {
            m_current_loops.pop();

            m_current_function--;
        }

    public:
        std::vector <std::vector <ir_instruction_t>>* get_functions() {
            return &m_functions;
        }

        void init(parser_t* parser, error_logger_t* logger, cli_parser_t* cli) {
            m_po = parser->get_output();

            _log(debug, "m_po->variables.size()=%u", m_po->variables.size());

            m_cli = cli;
            m_logger = logger;

            m_functions.resize(1);
        }

        uint32_t generate_impl(expression_t* expr, int base, bool pointer = false, bool inside_fn = false) {
            switch (expr->get_type()) {
                case EX_IF_ELSE: {
                    if_else_t* ie = (if_else_t*)expr;

                    int m_this_loop = m_current_loops.top()++;

                    generate_impl(ie->cond, base, false, inside_fn);

                    append({IR_CMPZB,
                        "EQ",
                        "R" + std::to_string(base),
                        (ie->else_expr ? "E" : "L") + std::to_string(m_this_loop)
                    });

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

                    int m_this_loop = m_current_loops.top()++;

                    // To-do: clean this up
                    append({IR_LABEL, "!L" + std::to_string(m_this_loop)});

                    generate_impl(wl->condition, base, false, inside_fn);

                    append({IR_CMPZB,
                        "EQ",
                        "R" + std::to_string(base),
                        "E" + std::to_string(m_this_loop)
                    });

                    generate_impl(wl->body, base, false, inside_fn);

                    append({IR_BRANCH, "AL", "L" + std::to_string(m_this_loop)});

                    append({IR_LABEL, "!E" + std::to_string(m_this_loop)});
                } break;

                case EX_FUNCTION_DEF: {
                    function_def_t* fd = (function_def_t*)expr;

                    begin_function();

                    m_current_num_locals.push(0);
                    m_current_num_args.push(0);

                    m_local_maps.push(m_dummy_local_map);

                    append({IR_LABEL, fd->name});

                    int arg_frame_pos = 1;

                    for (function_arg_t& arg : fd->args) {
                        m_current_num_args.top()++;
                        
                        append({IR_DEFINE, get_variable_name(arg.name), "[fp-" + std::to_string(4 * (arg_frame_pos++)) + "]"});

                        variable_t var;

                        var.address = m_current_num_args.top() * 4;
                        var.type = arg.type;

                        m_local_maps.top().insert({arg.name, var});
                    }

                    variable_t return_address;

                    return_address.address = m_current_num_args.top() * 4;
                    return_address.type = "u32";

                    m_current_num_args.top()++;

                    m_local_maps.top().insert({"<return_address>", return_address});

                    generate_impl(fd->body, base, false, true);

                    append({IR_MOV, "A0", "R" + std::to_string(base)});

                    if (m_current_num_locals.top()) {
                        append({IR_ADDSP, std::to_string(m_current_num_locals.top() * 4)});
                    }

                    for (function_arg_t& arg : fd->args) {
                        append({IR_UNDEF, get_variable_name(arg.name)});
                    }

                    append({IR_RET});

                    end_function();

                    m_current_num_locals.pop();
                    m_current_num_args.pop();

                    m_local_maps.pop();

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
                        m_current_num_locals.top()++;

                        variable_t var;

                        var.address = (m_current_num_locals.top() + m_current_num_args.top()) * 4;
                        var.type = vd->type;

                        m_local_maps.top().insert({vd->name, var});
                    }

                    return 1;
                } break;

                case EX_FUNCTION_CALL: {
                    function_call_t* fc = (function_call_t*)expr;

                    int r = generate_impl(fc->addr, base, true, inside_fn);

                    append({IR_PUSHR, "FP"});

                    for (expression_t* exp : fc->args) {
                        generate_impl(exp, base + r, false, inside_fn);

                        append({IR_PUSHR, "R" + std::to_string(base + r)});
                    }

                    append({IR_MOV, "FP", "SP"});
                    append({IR_ADDFP, std::to_string(fc->args.size() * 4)});

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

                    bool local = m_local_maps.top().contains(nr->name);

                    if (local) {
                        if (!pointer) {
                            // If referring by value, then load the value from
                            // stack

                            variable_t var = m_local_maps.top()[nr->name];

                            std::string size = std::to_string(types[var.type]);

                            append({IR_LOADF, "R" + std::to_string(base), std::to_string(var.address), size});
                        } else {
                            variable_t var = m_local_maps.top()[nr->name];

                            std::string size = std::to_string(types[var.type]);

                            // Else, load the address in stack 
                            append({IR_LEAF, "R" + std::to_string(base), std::to_string(var.address), size});
                        }
                    } else {
                        // If its a global variable, then load it's address
                        append({IR_MOVI, "R" + std::to_string(base), nr->name});

                        // If referring by value, then load the value at that
                        // address                        
                        if (!pointer) {
                            append({IR_LOADR, "R" + std::to_string(base), "R" + std::to_string(base), "4"});
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

                    if (aa->type_or_name->get_type() != EX_TYPE) {
                        binary_op_t* bo = new binary_op_t;

                        bo->lhs = aa->addr;
                        bo->rhs = aa->type_or_name;
                        bo->bop = "+";

                        int a = generate_impl(bo, base, false, inside_fn);

                        //if (!pointer) { 
                            append({IR_LOADR, "R" + std::to_string(base), "R" + std::to_string(base)});
                        //}

                        return a;
                    } else {
                        int addr = generate_impl(aa->addr, base, false, inside_fn);

                        if (!pointer) { 
                            append({IR_LOADR, "R" + std::to_string(base), "R" + std::to_string(base)});
                        }

                        return addr;
                    }
                } break;

                case EX_ASSIGNMENT: {
                    assignment_t* ae = (assignment_t*)expr;

                    ae->assignee->get_type();

                    int rhs = generate_impl(ae->value, base, false, inside_fn);
                    int lhs = generate_impl(ae->assignee, base + rhs, true, inside_fn);

                    append({IR_STORE, "R" + std::to_string(base + rhs), "R" + std::to_string(base)});

                    return lhs + rhs;
                } break;

                case EX_ARRAY: {
                    std::string label = new_array(*((array_t*)expr));

                    append({IR_MOVI, "R" + std::to_string(base), label});

                    return 1;
                } break;

                case EX_BLOB: {
                    blob_t* blob = (blob_t*)expr;

                    std::string label = new_blob(blob->file);

                    append({IR_MOVI, "R" + std::to_string(base), label});

                    return 1;
                } break;
                
                default: return 1;
            }

            return 0;
        }

        void generate() {
            if (m_cli->get_setting(ST_OUTPUT_FORMAT) == "elf32") {
                m_functions.front().push_back({IR_ENTRY, "<ENTRY>"});

                m_functions.front().push_back({IR_ORG, "0x40000"});
                m_functions.front().push_back({IR_SECTION, ".text"});
            }

            m_functions.front().push_back({IR_LABEL, "<ENTRY>"});

            for (expression_t* expr : m_po->source) {
                generate_impl(expr, 0);
            }
            
            // Function call semantics
            m_functions.front().push_back({IR_PUSHR, "FP"});
            m_functions.front().push_back({IR_MOV, "FP", "SP"});
            m_functions.front().push_back({IR_MOVI, "R0", "F<global>.main"});
            m_functions.front().push_back({IR_CALLR, "R0"});
            m_functions.front().push_back({IR_MOV, "R0", "A0"});
            m_functions.front().push_back({IR_MOV, "SP", "FP"});
            m_functions.front().push_back({IR_POPR, "FP"});

            // Debug program end software breakpoint
            m_functions.front().push_back({IR_DEBUG, "0xdeadc0de"});

            // Section padding
            m_functions.back().push_back({IR_NOP});
            m_functions.back().push_back({IR_NOP});

            int i = 0;

            // Align .rodata to 4-byte boundary
            m_functions.back().push_back({IR_ALIGN, "4"});

            if (m_cli->get_setting(ST_OUTPUT_FORMAT) == "elf32") {
                m_functions.back().push_back({IR_SECTION, ".rodata"});
            }


            for (array_t& arr : m_pending_arrays) {
                m_functions.back().push_back({IR_LABEL, "DA" + std::to_string(i++)});
                
                for (expression_t* expr : arr.values) {
                    switch (expr->get_type()) {
                        case EX_NUMERIC_LITERAL: {
                            numeric_literal_t* nl = (numeric_literal_t*)expr;

                            m_functions.back().push_back({IR_DEFV, "l", std::to_string(nl->value)});
                        } break;

                        case EX_STRING_LITERAL: {
                            string_literal_t* sl = (string_literal_t*)expr;

                            std::string label = new_string(sl->str);

                            m_functions.back().push_back({IR_DEFV, "l", label});
                        } break;

                        case EX_NAME_REF: {
                            name_ref_t* nr = (name_ref_t*)expr;
                            
                            m_functions.back().push_back({IR_DEFV, "l", nr->name});
                        } break;

                        case EX_FUNCTION_DEF: {
                            function_def_t* fd = (function_def_t*)expr;

                            m_functions.back().push_back({IR_DEFV, "l", fd->name});
                        } break;

                        default: {
                            _log(error, "Non-compile-time expressions aren't allowed on arrays");
                        } break;
                    }
                }
            }
            
            for (string_t& str : m_pending_strings) {
                m_functions.back().push_back({IR_LABEL, str.name});
                m_functions.back().push_back({IR_DEFSTR, str.value});
            }
            
            for (blob_def_t& blob : m_pending_blobs) {
                m_functions.back().push_back({IR_LABEL, blob.name});
                m_functions.back().push_back({IR_DEFBLOB, blob.file});
            }

            // Align assembler generated sections to 4-byte boundary
            m_functions.back().push_back({IR_ALIGN, "4"});
        }
    };
}