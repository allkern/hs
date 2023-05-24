#pragma once

#include "parser.hpp"

#include "expressions/expression_block.hpp"
#include "expressions/numeric_literal.hpp"
#include "expressions/numeric_literal_parser.hpp"
#include "expressions/string_literal.hpp"
#include "expressions/function_call.hpp"
#include "expressions/array_access.hpp"
#include "expressions/variable_def.hpp"
#include "expressions/function_def.hpp"
#include "expressions/function_def_parser.hpp"
#include "expressions/assignment.hpp"
#include "expressions/while_loop.hpp"
#include "expressions/asm_block.hpp"
#include "expressions/binary_op.hpp"
#include "expressions/name_ref.hpp"
#include "expressions/unary_op.hpp"
#include "expressions/comp_op.hpp"
#include "expressions/if_else.hpp"
#include "expressions/return.hpp"
#include "expressions/invoke.hpp"
#include "expressions/array.hpp"
#include "expressions/blob.hpp"

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "parser", \
        msg, \
        m_current.line, m_current.offset, m_current.text.size() \
    ); \
    std::exit(1);

namespace hs {
    typedef expression_t* (*expression_parser_t)(parser_t*, expression_t*);

    std::unordered_map <expression_type_t, expression_parser_t> g_expression_parsers = {
        { EX_FUNCTION_DEF   , parse_function_def    },
        { EX_NUMERIC_LITERAL, parse_numeric_literal }
    };

    error_logger_t* parser_t::get_logger() {
        return m_logger;
    }

    void parser_t::init(stream_t <lexer_token_t>* input, error_logger_t* logger) {
        m_input = input;
        m_logger = logger;

        m_ts.init();
    }

    parser_output_t* parser_t::get_output() {
        return &m_output;
    }

    bool parser_t::is_token(lexer_token_type_t lt) {
        return m_current.type == lt;
    }

    bool parser_t::expect_token(lexer_token_type_t lt) {
        if (!is_token(lt)) {
            ERROR(fmt("Expected " ESCAPE(37) "%s" ESCAPE(0) ", got " ESCAPE(37) "\'%s\'" ESCAPE(0) " instead",
                lexer_token_hr_names[lt],
                m_current.text.c_str()
            ));

            std::exit(1);
        }

        return true;
    }

    expression_t* parser_t::expect_expr(expression_type_t et, expression_t* arg = nullptr) {
        expression_parser_t p = g_expression_parsers[et];

        assert(p);

        expression_t* expr = p(this, arg);

        if (!expr) {
            ERROR(fmt("Expected " ESCAPE(37) "\'%s\'", parser_expression_hr_names[et]));

            return nullptr;
        }

        return expr;
    }

    expression_t* parser_t::try_expr(expression_type_t et, expression_t* arg = nullptr) {
        expression_parser_t p = g_expression_parsers[et];

        assert(p);

        size_t pos = m_input->tellg();

        expression_t* expr = p(this, arg);

        if (!expr) {
            m_input->seekg(pos);

            return nullptr;
        }

        return expr;
    }

    void parser_t::init_expr(expression_t* expr) {
        expr->line = m_current.line;
        expr->offset = m_current.offset;
        expr->len = m_current.text.size();
    }

    std::string parser_t::get_anonymous_function_name() {
        std::string name = "<anonymous_";

        name += std::to_string(m_anonymous_functions++);
        name += ">";

        return name;
    }

    type_system_t* parser_t::type_system() {
        return &m_ts;
    }

    bool parser_t::is_type() {
        return  m_ts.exists(m_current.text) ||
                is_type_modifier() ||
                (m_current.type == LT_KEYWORD_FN);
    }

    void parser_t::consume() {
        m_current = m_input->get();
    }

    inline lexer_token_t parser_t::current() {
        return m_current;
    }

    expression_t* parser_t::parse_expression_impl() {
        expression_t* expr;

        switch (m_current.type) {
            case LT_KEYWORD_FN: {
                // Definition (function type)
                if (m_input->peek().type == LT_STAR) {
                    hs_type_t* type = parse_type();
                } else {
                    expr = expect_expr(EX_FUNCTION_DEF);
                }
            } break;

            case LT_IDENT: {
                if (!is_type()) {
                    // Name ref
                } else {
                    // Definition
                    hs_type_t* type = parse_type();

                    // Parse remaining stuff
                }
            } break;

            case LT_LITERAL_NUMERIC: {
                expr = expect_expr(EX_NUMERIC_LITERAL);
            } break;

            case LT_KEYWORD_MUT: case LT_KEYWORD_STATIC:
            case LT_KEYWORD_TYPEDEF: {
                hs_type_t* type = parse_type();

                expr = nullptr;
            } break;

            default: {
                ERROR(fmt("Unhandled token \"" ESCAPE(37;1) "%s" ESCAPE(0) "\"", m_current.text.c_str()));

                expr = nullptr;
            };
        }

        return expr;
    }

    inline bool parser_t::is_type_modifier() {
        return (m_current.type == LT_KEYWORD_MUT   ) ||
               (m_current.type == LT_KEYWORD_STATIC) ||
               (m_current.type == LT_KEYWORD_CONST );
    }

    function_type_t* parser_t::parse_function_type() {
        std::string signature;

        if (m_current.type != LT_KEYWORD_FN) {
            return nullptr;
        }

        function_type_t* fnt = new function_type_t;

        consume();

        if (m_current.type != LT_STAR) {
            // Error: expected * after fn on function pointer
            // definition

            return nullptr;
        }

        consume();

        switch (m_current.type) {
            case LT_ARROW: {
                consume();

                fnt->return_type = parse_type();
                fnt->signature = get_signature(fnt);

                return fnt;
            } break;

            case LT_OPENING_PARENT: {
                consume();

                while (m_current.type != LT_CLOSING_PARENT) {
                    definition_t def = parse_definition();

                    fnt->args.push_back(def);

                    switch (m_current.type) {
                        case LT_COMMA: {
                            consume();
                        } break;

                        default: {
                            // Error: expected , or )
                        } break;
                    }
                }

                consume();

                if (m_current.type != LT_ARROW) {
                    // Error: expected return type

                    return nullptr;
                }

                consume();

                fnt->return_type = parse_type();
                fnt->signature = get_signature(fnt);

                return fnt;
            } break;
        }

        return nullptr;
    }

    hs_type_t* parser_t::parse_type() {
        std::string signature;

        bool mut, is_static;

        hs_type_t* type;

        while (is_type_modifier()) {
            switch (m_current.type) {
                case LT_KEYWORD_MUT   : mut = true; break;
                case LT_KEYWORD_STATIC: is_static = true; break;
                case LT_KEYWORD_CONST : mut = false; break;
            }

            signature.append(m_current.text);
            signature.push_back(' ');

            m_current = m_input->get();
        }

        // Function pointer
        if (m_current.type == LT_KEYWORD_FN) {
            function_type_t* fty = parse_function_type();

            if (m_ts.exists(fty->signature)) {
                std::string fty_sig = fty->signature;

                delete fty;

                return m_ts.get_type(fty_sig);
            } else {
                m_ts.add_type(fty->signature, fty);

                return fty;
            }
        }

        if (m_current.type == LT_KEYWORD_TYPEDEF) {
            m_current = m_input->get();

            hs_type_t* type = parse_type();

            if (m_current.type != LT_IDENT) {
                ERROR("Expected typedef alias");
                // Error: Expected typedef name

                return nullptr;
            }

            m_ts.type_def(m_current.text, type->signature);

            m_current = m_input->get();

            return type;
        }

        if (!m_ts.exists(m_current.text)) {
            // Error: expected a type after type modifier

            return nullptr;
        }

        type = m_ts.get_type(m_current.text);

        signature.append(m_current.text);

        m_current = m_input->get();

        while (m_current.type == LT_STAR) {
            type = new pointer_type_t(type);

            signature.push_back('*');

            m_current = m_input->get();
        }

        type->mut = mut;
        type->is_static = is_static;

        _log(debug, "signature=%s (%s)", signature.c_str(), type->signature.c_str());

        m_ts.add_type(signature, type);

        return type;
    }

    definition_t parser_t::parse_definition() {
        definition_t def;

        if (is_type()) {
            def.type = parse_type();

            return def;
        } else {
            if (m_current.type != LT_IDENT) {
                // Error: Expected name or type

                return def;
            }

            def.name = m_current.text;

            m_current = m_input->get();

            if (m_current.type != LT_COLON) {
                // Error: Expected :

                return def;
            }

            m_current = m_input->get();

            if (!is_type()) {
                // Error: Expected type

                return def;
            }

            def.type = parse_type();
        }

        return def;
    }

    // -- Support functions --

    expression_t* parser_t::parse_rhs(expression_t* lhs) {
        expression_t* expr = lhs;

        if (m_current.type == LT_OPERATOR_UNARY) {
            unary_op_t* uo = new unary_op_t;

            uo->op = m_current.text;
            uo->post = true;
            uo->operand = lhs;

            m_current = m_input->get();

            expr = uo;
        }

        if ((m_current.type == LT_OPERATOR_BINARY) || (m_current.type == LT_STAR) || (m_current.type == LT_AMPERSAND)) {
            binary_op_t* bo = new binary_op_t;

            bo->op = m_current.text;
            bo->lhs = lhs;

            m_current = m_input->get();

            bo->rhs = parse_expression();

            expr = bo;
        }

        if (m_current.type == LT_OPERATOR_COMP) {
            comp_op_t* co = new comp_op_t;

            co->op = m_current.text;
            co->lhs = lhs;

            m_current = m_input->get();

            co->rhs = parse_expression();

            expr = co;
        }

        if (m_current.type == LT_OPERATOR_ASSIGN) {
            assignment_t* as = new assignment_t;

            as->assignee = lhs;
            as->op = m_current.text;

            m_current = m_input->get();

            as->value = parse_expression();

            expr = as;
        }

        if (m_current.type == LT_OPENING_PARENT) {
            function_call_t* fc = new function_call_t;

            m_current = m_input->get();

            // To-do
            //expr = parse_function_call(lhs);
        }

        if (m_current.type == LT_OPENING_BRACKET) {
            array_access_t* aa = new array_access_t;

            m_current = m_input->get();

            // To-do
            //expr = parse_array_access(lhs);
        }

        return expr;
    }

    expression_t* parser_t::parse_expression() {
        expression_t* expr = nullptr;

        bool parenthesized = m_current.type == LT_OPENING_PARENT;

        if (parenthesized) {
            m_current = m_input->get();

            expr = parse_expression();

            if (m_current.type != LT_CLOSING_PARENT) {
                ERROR("Expected ')'");
            }

            m_current = m_input->get();

            expr = parse_rhs(expr);
        } else {
            // Parse pre-unary op
            if (m_current.type == LT_OPERATOR_UNARY) {
                unary_op_t* uo = new unary_op_t;

                uo->op = m_current.text;
                uo->post = false;

                m_current = m_input->get();

                uo->operand = parse_expression();

                expr = uo;
            } else {
                expr = parse_expression_impl();
            }

            expr = parse_rhs(expr);
        }

        return expr;
    }

    bool parser_t::parse() {
        m_current = m_input->get();

        while (m_current.type != LT_NONE) {
            expression_t* expr = parse_expression();

            if (m_current.type != LT_SEMICOLON) {
                // Error, expected semicolon
                ERROR("Expected \';\' before expression");
            }

            m_current = m_input->get();

            m_output.source.push_back(expr);
        }

        return true;
    }
}

#undef ERROR