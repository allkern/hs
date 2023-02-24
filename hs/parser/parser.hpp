#pragma once

#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"

#include "../stream.hpp"
#include "../error.hpp"

#include "expression.hpp"
#include "output.hpp"

#include "expressions/expression_block.hpp"
#include "expressions/numeric_literal.hpp"
#include "expressions/string_literal.hpp"
#include "expressions/function_call.hpp"
#include "expressions/array_access.hpp"
#include "expressions/variable_def.hpp"
#include "expressions/function_def.hpp"
#include "expressions/assignment.hpp"
#include "expressions/while_loop.hpp"
#include "expressions/asm_block.hpp"
#include "expressions/binary_op.hpp"
#include "expressions/name_ref.hpp"
#include "expressions/comp_op.hpp"
#include "expressions/if_else.hpp"
#include "expressions/invoke.hpp"
#include "expressions/array.hpp"
#include "expressions/type.hpp"
#include "expressions/blob.hpp"

#include <cassert>

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "parser", \
        msg, \
        m_current.line, m_current.offset, m_current.text.size() \
    ); \
    return nullptr;

namespace hs {
    class parser_t {
        stream_t <lexer_token_t>* m_input;
        error_logger_t* m_logger;

        lexer_token_t m_current;
        parser_output_t m_output;

        int m_anonymous_functions = 0;

        std::string get_anonymous_function_name() {
            std::string name = "<anonymous_";

            name += std::to_string(m_anonymous_functions++);
            name += ">";

            return name;
        }
    
    public:
        void init(stream_t <lexer_token_t>* input, error_logger_t* logger) {
            m_input = input;
            m_logger = logger;
        }

        bool is_type(std::string ident) {
            return types.contains(ident) || type_aliases.contains(ident); 
        }

        expression_t* parse_expression_impl();
        expression_t* parse_expression();

        expression_t* parse_function_definition() {
            if (m_current.type != LT_KEYWORD_FN) {
                assert(false); // ??
            }

            function_def_t* def = new function_def_t;

            def->line = m_current.line;
            def->offset = m_current.offset;
            def->len = m_current.text.size();

            m_current = m_input->get();

            switch (m_current.type) {
                case LT_ARROW: {
                    def->name = get_anonymous_function_name();

                    goto parse_function_type;
                } break;
                
                case LT_COLON: {
                    def->name = get_anonymous_function_name();
                    def->type = "<any>";

                    goto parse_function_body;
                } break;

                case LT_OPENING_PARENT: {
                    def->name = get_anonymous_function_name();

                    goto parse_function_args;
                } break;

                default: break;
            }

            if (m_current.type != LT_IDENT) {
                ERROR("Expected " ESCAPE(37;1) "function name" ESCAPE(0) " after " ESCAPE(37;1) "fn" ESCAPE(0) " keyword");
            }

            def->name = m_current.text;

            parse_remaining_prototype:

            m_current = m_input->get();

            switch (m_current.type) {
                case LT_OPENING_PARENT: {
                    // Parse arguments
                    parse_function_args:

                    function_arg_t arg;

                    parse_arg:

                    m_current = m_input->get();

                    switch (m_current.type) {
                        case LT_IDENT: goto do_parse_arg;
                        case LT_CLOSING_PARENT: goto parse_remaining_prototype;
                        default: {
                            ERROR("Expected " ESCAPE(37;1) "argument name" ESCAPE(0));
                        } break;
                    }

                    do_parse_arg:

                    arg.name = m_current.text;

                    m_current = m_input->get();

                    if (m_current.type != LT_COLON) {
                        ERROR("Expected \'" ESCAPE(37;1) ":" ESCAPE(0) "\' after name");
                    }

                    m_current = m_input->get();

                    if (m_current.type != LT_IDENT) {
                        ERROR("Expected " ESCAPE(37;1) "argument type" ESCAPE(0));
                    }

                    if (!is_type(m_current.text)) {
                        ERROR(fmt("Identifier \"" ESCAPE(37;1) "%s" ESCAPE(0) "\" does not name a type", m_current.text.c_str()));
                    }

                    arg.type = m_current.text;

                    m_current = m_input->get();

                    switch (m_current.type) {
                        case LT_COMMA: {
                            def->args.push_back(arg);

                            goto parse_arg;
                        } break;

                        case LT_CLOSING_PARENT: {
                            def->args.push_back(arg);

                            goto parse_remaining_prototype;
                        } break;

                        default: {
                            ERROR("Expected \'" ESCAPE(37;1) "," ESCAPE(0) "\' or \'" ESCAPE(37;1) ")" ESCAPE(0) "\'");
                        } break;
                    }
                } break;

                case LT_ARROW: {
                    parse_function_type:

                    // Parse function type
                    m_current = m_input->get();

                    if (m_current.type != LT_IDENT) {
                        ERROR("Expected " ESCAPE(37;1) "type" ESCAPE(0) " after " ESCAPE(37;1) "->" ESCAPE(0) " on function definition");
                    }

                    if (!is_type(m_current.text)) {
                        ERROR(fmt("Identifier \"" ESCAPE(37;1) "%s" ESCAPE(0) "\" does not name a type", m_current.text.c_str()));
                    }

                    def->type = m_current.text;

                    goto parse_remaining_prototype;
                } break;

                case LT_COLON: {
                    parse_function_body:

                    m_current = m_input->get();

                    def->body = parse_expression();

                    if (!def->body) return nullptr;
                } break;

                default: {
                    ERROR("Expected " ESCAPE(37;1) "parameters" ESCAPE(0) " or " ESCAPE(37;1) "type" ESCAPE(0) " after " ESCAPE(37;1) "function name" ESCAPE(0));
                } break;
            }

            return def;
        }

        expression_t* parse_numeric_literal() {
            if (m_current.type != LT_LITERAL_NUMERIC) {
                assert(false); // ??
            }

            numeric_literal_t* num = new numeric_literal_t;

            num->line = m_current.line;
            num->offset = m_current.offset;
            num->len = m_current.text.size();

            if (!std::isdigit(m_current.text[0])) {
                // This is a char literal
                
                num->value = m_current.text[0];
            } else {
                num->value = std::stoull(m_current.text, nullptr, 0);
            }

            m_current = m_input->get();

            return num;
        }

        expression_t* parse_string_literal() {
            if (m_current.type != LT_LITERAL_STRING) {
                assert(false); // ??
            }

            string_literal_t* str = new string_literal_t;

            str->line = m_current.line;
            str->offset = m_current.offset;
            str->len = m_current.text.size();

            str->str = m_current.text;

            m_current = m_input->get();

            return str;
        }

        expression_t* parse_invoke() {
            if (m_current.type != LT_KEYWORD_INVOKE) {
                assert(false); // ??
            }

            invoke_expr_t* invoke = new invoke_expr_t;

            invoke->line = m_current.line;
            invoke->offset = m_current.offset;
            invoke->len = m_current.text.size();

            m_current = m_input->get();

            invoke->ptr = parse_expression();

            return invoke;
        }

        expression_t* parse_binary_op(expression_t* lhs) {
            binary_op_t* bop = new binary_op_t;

            bop->line = m_current.line;
            bop->offset = m_current.offset;
            bop->len = m_current.text.size();

            bop->op = m_current.text;

            m_current = m_input->get();
            
            bop->lhs = lhs;
            bop->rhs = parse_expression();

            return bop;
        }

        expression_t* parse_comp_op(expression_t* lhs) {
            comp_op_t* cop = new comp_op_t;

            cop->line = m_current.line;
            cop->offset = m_current.offset;
            cop->len = m_current.text.size();

            cop->op = m_current.text;

            m_current = m_input->get();
            
            cop->lhs = lhs;
            cop->rhs = parse_expression();

            return cop;
        }

        expression_t* parse_variable_def(std::string type) {
            variable_def_t* var = new variable_def_t;

            var->line = m_current.line;
            var->offset = m_current.offset;
            var->len = m_current.text.size();

            var->type = type;
            var->name = m_current.text;

            m_current = m_input->get();

            return var;
        }

        expression_t* parse_blob() {
            blob_t* blob = new blob_t;

            if (m_current.type != LT_KEYWORD_BLOB) {
                assert(false); // ??
            }

            blob->line = m_current.line;
            blob->offset = m_current.offset;
            blob->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_LITERAL_STRING) {
                ERROR("Expected string literal after blob");
            }

            blob->file = m_current.text;

            m_current = m_input->get();

            return blob;
        }

        expression_t* parse_array() {
            array_t* arr = new array_t;

            if (m_current.type != LT_KEYWORD_ARRAY) {
                assert(false); // ??
            }

            arr->line = m_current.line;
            arr->offset = m_current.offset;
            arr->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_OPENING_BRACKET) {
                ERROR("Expected [ after array");
            }

            m_current = m_input->get();

            if (m_current.type != LT_IDENT) {
                ERROR("Expected type after [");
            }

            if (!is_type(m_current.text)) {
                ERROR(fmt("Identifier \"" ESCAPE(37;1) "%s" ESCAPE(0) "\" does not name a type", m_current.text.c_str()));
            }

            arr->type = type_t();
            arr->type.type = m_current.text;

            m_current = m_input->get();

            if (m_current.type == LT_COMMA) {
                m_current = m_input->get();

                if (m_current.type != LT_LITERAL_NUMERIC) {
                    ERROR("Expected numeric literal after [");
                }

                if (!std::isdigit(m_current.text[0])) {
                    // This is a char literal
                    arr->size = m_current.text[0];
                } else {
                    arr->size = std::stoull(m_current.text, nullptr, 0);
                }

                m_current = m_input->get();
            }

            if (m_current.type != LT_CLOSING_BRACKET) {
                ERROR("Expected ] after array type or size");
            }

            m_current = m_input->get();

            if (m_current.type == LT_OPENING_BRACE) {
                expression_t* lhs;
                expression_t* op;

                m_current = m_input->get();

                unsigned int real_size = 0;

                while (m_current.type != LT_CLOSING_BRACE) {
                    lhs = parse_expression();

                    do {
                        op = parse_rightside_operation(lhs);

                        if (op) lhs = op;
                    } while (op);

                    arr->values.push_back(lhs);
                    real_size++;

                    if (m_current.type == LT_CLOSING_BRACE) break;

                    if (m_current.type != LT_COMMA) {
                        ERROR("Expressions on arrays must be separated by commas");
                    }

                    m_current = m_input->get();
                }

                if (m_current.type == LT_CLOSING_BRACE) {
                    if ((real_size != arr->size) && (arr->size != 0)) {
                        if (m_logger) m_logger->print_warning(
                            "parser",
                            "Array size doesn't match declared size",
                            m_current.line, m_current.offset, m_current.text.size(), true
                        );
                    }

                    arr->size = real_size;
                }

                m_current = m_input->get();
            }

            if (!arr->size) {
                ERROR("Cannot define zero-sized arrays");
            }

            return arr;
        }

        expression_t* parse_name_ref(std::string name) {
            name_ref_t* ref = new name_ref_t;

            ref->line = m_current.line;
            ref->offset = m_current.offset;
            ref->len = m_current.text.size();
            
            ref->name = name;

            return ref;
        }

        expression_t* parse_if_else() {
            if (m_current.type != LT_KEYWORD_IF) {
                assert(false); // ??
            }

            if_else_t* ifl = new if_else_t;

            ifl->line = m_current.line;
            ifl->offset = m_current.offset;
            ifl->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_OPENING_PARENT) {
                ERROR("Expected opening parenthesis after if");
            }

            m_current = m_input->get();

            ifl->cond = parse_expression();

            if (m_current.type != LT_CLOSING_PARENT) {
                ERROR("Expected closing parenthesis after if condition");
            }

            m_current = m_input->get();

            if (m_current.type != LT_COLON) {
                ERROR("Expected colon after closing parenthesis");
            }

            m_current = m_input->get();

            ifl->if_expr = parse_expression();

            if (m_current.type == LT_KEYWORD_ELSE) {
                m_current = m_input->get();

                if (m_current.type != LT_COLON) {
                    ERROR("Expected colon after else");
                }

                m_current = m_input->get();

                ifl->else_expr = parse_expression();
            }

            return ifl;
        }

        expression_t* parse_while_loop() {
            if (m_current.type != LT_KEYWORD_WHILE) {
                assert(false); // ??
            }

            while_loop_t* whl = new while_loop_t;

            whl->line = m_current.line;
            whl->offset = m_current.offset;
            whl->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_OPENING_PARENT) {
                ERROR("Expected opening parenthesis after while loop declaration");
            }

            m_current = m_input->get();

            whl->condition = parse_expression();

            if (m_current.type != LT_CLOSING_PARENT) {
                ERROR("Expected closing parenthesis after while loop condition");
            }

            m_current = m_input->get();

            if (m_current.type != LT_COLON) {
                ERROR("Expected colon after closing parenthesis");
            }

            m_current = m_input->get();

            whl->body = parse_expression();

            return whl;
        }

        expression_t* parse_array_access(expression_t* lhs) {
            array_access_t* access = new array_access_t;

            access->line = m_current.line;
            access->offset = m_current.offset;
            access->len = m_current.text.size();

            access->type_or_name = lhs;

            access->addr = parse_expression();

            if (m_current.type != LT_CLOSING_BRACKET) {
                ERROR("Expected closing bracket on array access expression");

                return nullptr;
            }

            m_current = m_input->get();

            return access;
        }

        expression_t* parse_function_call(expression_t* addr) {
            function_call_t* call = new function_call_t;

            call->line = m_current.line;
            call->offset = m_current.offset;
            call->len = m_current.text.size();

            call->addr = addr;

            if (m_current.type == LT_CLOSING_PARENT) {
                m_current = m_input->get();

                return call;
            }

            while (m_current.type != LT_CLOSING_PARENT) {
                call->args.push_back(parse_expression());

                if (m_current.type == LT_CLOSING_PARENT) {
                    m_current = m_input->get();

                    break;
                }

                if (m_current.type != LT_COMMA) {
                    ERROR("Expected comma after call argument");
                }

                m_current = m_input->get();
            }

            return call;
        }

        expression_t* parse_assignment(expression_t* lhs) {
            assignment_t* assign = new assignment_t;

            assign->line = m_current.line;
            assign->offset = m_current.offset;
            assign->len = m_current.text.size();

            assign->assignee = lhs;
            assign->op = m_current.text;

            m_current = m_input->get();

            assign->value = parse_expression();

            return assign; 
        }

        expression_t* parse_asm_block() {
            asm_block_t* asm_block = new asm_block_t;

            asm_block->line = m_current.line;
            asm_block->offset = m_current.offset;
            asm_block->len = m_current.text.size();
            asm_block->assembly = m_current.text;

            m_current = m_input->get();

            return asm_block;
        }

        expression_t* parse_expression_block() {
            expression_block_t* block = new expression_block_t;

            block->line = m_current.line;
            block->offset = m_current.offset;
            block->len = m_current.text.size();

            expression_t* lhs;
            expression_t* op;

            m_current = m_input->get();

            while (m_current.type != LT_CLOSING_BRACE) {
                lhs = parse_expression();

                do {
                    op = parse_rightside_operation(lhs);

                    if (op) lhs = op;
                } while (op);

                block->block.push_back(lhs);

                if (m_current.type != LT_SEMICOLON) {
                    ERROR("Expressions on blocks must be separated by semicolons");
                }

                m_current = m_input->get();
            }

            m_current = m_input->get();

            return block;
        }

        expression_t* parse_rightside_operation(expression_t* lhs) {
            switch (m_current.type) {
                case LT_OPERATOR_BINARY: case LT_STAR: case LT_AMPERSAND: {
                    return parse_binary_op(lhs);
                } break;

                case LT_OPERATOR_COMP: {
                    return parse_comp_op(lhs);
                } break;

                case LT_OPENING_PARENT: {
                    m_current = m_input->get();

                    return parse_function_call(lhs);
                } break;

                case LT_OPENING_BRACKET: {
                    m_current = m_input->get();

                    return parse_array_access(lhs);
                } break;

                case LT_OPERATOR_ASSIGN: {
                    return parse_assignment(lhs);
                } break;

                default: return nullptr;
            }

            return nullptr;
        }

        parser_output_t* get_output() {
            return &m_output;
        }

        bool parse() {
            expression_t* lhs;
            expression_t* op;

            m_current = m_input->get();

            while (m_current.type != LT_NONE) {
                lhs = parse_expression();

                do {
                    op = parse_rightside_operation(lhs);

                    if (op) lhs = op;
                } while (op);

                // Handle ; thing
                if (lhs == (expression_t*)10) {
                    m_current = m_input->get();

                    continue;
                }

                if (lhs) {
                    m_output.source.push_back(lhs);

                    //_log(debug, "expression:\n%s", lhs->print(0).c_str());
                } else {
                    return false;
                }

                m_current = m_input->get();
            }

            return true;
        }
    };
}

hs::expression_t* hs::parser_t::parse_expression_impl() {
    hs::expression_t* expr;
 
    switch (m_current.type) {
        case LT_KEYWORD_FN: {
            expr = parse_function_definition();
        } break;

        case LT_KEYWORD_WHILE: {
            expr = parse_while_loop();
        } break;

        case LT_KEYWORD_ARRAY: {
            expr = parse_array();
        } break;

        case LT_KEYWORD_BLOB: {
            expr = parse_blob();
        } break;

        case LT_KEYWORD_IF: {
            expr = parse_if_else();
        } break;

        case LT_LITERAL_NUMERIC: {
            expr = parse_numeric_literal();
        } break;

        case LT_LITERAL_STRING: {
            expr = parse_string_literal();
        } break;

        case LT_KEYWORD_INVOKE: {
            expr = parse_invoke();
        } break;

        case LT_OPENING_BRACKET: {
            type_t* none = new type_t;

            none->line = m_current.line;
            none->offset = m_current.offset;
            none->len = m_current.text.size();

            none->type = "none";

            m_current = m_input->get();

            expr = parse_array_access(none);
        } break;

        case LT_IDENT: {
            bool type = is_type(m_current.text);

            if (type) {
                std::string type = m_current.text;

                m_current = m_input->get();

                if (m_current.type == LT_IDENT) {
                    variable_def_t* var = new variable_def_t;

                    var->line = m_current.line;
                    var->offset = m_current.offset;
                    var->len = m_current.text.size();

                    var->type = type;
                    var->name = m_current.text;

                    expr = var;

                    m_current = m_input->get();
                } else {
                    type_t* type_expr = new type_t;

                    type_expr->line = m_current.line;
                    type_expr->offset = m_current.offset;
                    type_expr->len = m_current.text.size();

                    type_expr->type = type;

                    expr = type_expr;
                }
            } else {
                name_ref_t* name = new name_ref_t;

                name->line = m_current.line;
                name->offset = m_current.offset;
                name->len = m_current.text.size();

                name->name = m_current.text;

                expr = name;

                m_current = m_input->get();
            }
        } break;

        case LT_OPENING_BRACE: {
            expr = parse_expression_block();
        } break;

        case LT_ASM_BLOCK: {
            expr = parse_asm_block();
        } break;

        case LT_SEMICOLON: {
            return (hs::expression_t*)10;
        } break;

        default: {
            ERROR(fmt("Unhandled token \"" ESCAPE(37;1) "%s" ESCAPE(0) "\"", m_current.text.c_str()));

            expr = nullptr;
        };
    }

    return expr;
}

hs::expression_t* hs::parser_t::parse_expression() {
    expression_t* lhs;
    expression_t* op;

    bool parenthesized = m_current.type == LT_OPENING_PARENT;

    if (parenthesized) {
        parenthesized = true;

        m_current = m_input->get();

        lhs = hs::parser_t::parse_expression();
    } else {
        lhs = hs::parser_t::parse_expression_impl();
    }

    do {
        op = parse_rightside_operation(lhs);

        if (op) lhs = op;
    } while (op);
    
    if (parenthesized) {
        if (m_current.type != LT_CLOSING_PARENT) {
            ERROR("Expected \'" ESCAPE(37;1) ")" ESCAPE(0)"\'");

            return nullptr;
        } else {
            m_current = m_input->get();
        }
    }

    return lhs;
}

#undef ERROR