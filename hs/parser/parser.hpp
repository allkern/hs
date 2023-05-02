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
#include "expressions/unary_op.hpp"
#include "expressions/comp_op.hpp"
#include "expressions/if_else.hpp"
#include "expressions/return.hpp"
#include "expressions/invoke.hpp"
#include "expressions/array.hpp"
#include "expressions/blob.hpp"

#include "type_system.hpp"

#include <cassert>

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "parser", \
        msg, \
        m_current.line, m_current.offset, m_current.text.size() \
    ); \

namespace hs {
    static const char* parser_expression_hr_names[] = {
        "function-def"
    };

    enum parser_expression_t : int {
        PT_FUNCTION_DEF
    };

    class parser_t {
        stream_t <lexer_token_t>*   m_input;
        parser_output_t             m_output;
        error_logger_t*             m_logger;
        lexer_token_t               m_current;
        type_system_t               m_ts;

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

            m_ts.init();
        }

        typedef expression_t* (parser_t::*expression_parser_t)(expression_t*);

        std::unordered_map <parser_expression_t, expression_parser_t> m_expression_parsers = {
            { PT_FUNCTION_DEF, parse_function_def }
        };

        parser_output_t* get_output() {
            return &m_output;
        }

        inline bool is_token(lexer_token_type_t lt) {
            return m_current.type == lt;
        }

        bool expect_token(lexer_token_type_t lt) {
            if (!is_token(lt)) {
                ERROR(fmt("Expected " ESCAPE(37) "%s" ESCAPE(0) ", got " ESCAPE(37) "\'%s\'" ESCAPE(0) " instead",
                    lexer_token_hr_names[lt],
                    m_current.text.c_str()
                ));

                std::exit(1);
            }

            return true;
        }

        expression_t* expect_expr(parser_expression_t pt, expression_t* arg) {
            expression_parser_t p = m_expression_parsers[pt];

            assert(p);

            expression_t* expr = p(arg);

            if (!expr) {
                ERROR(fmt("Expected " ESCAPE(37) "\'%s\'", parser_expression_hr_names[pt]));

                return false;
            }

            return expr;
        }

        expression_t* try_expr(parser_expression_t pt, expression_t* arg) {
            expression_parser_t p = m_expression_parsers[pt];

            assert(p);

            size_t pos = m_input->tellg();

            expression_t* expr = p(arg);

            if (!expr) {
                m_input->seekg(pos);

                return nullptr;
            }

            return expr;
        }

        inline void consume() {
            m_current = m_input->get();
        }
    
        // fn: expr
        // fn name: expr
        // fn (arg-list): expr
        // fn -> type: expr
        // fn name(arg-list): expr
        // fn name -> type: expr
        // fn (arg-list) -> type: expr
        expression_t* parse_function_def(expression_t* prev = nullptr) {
            if (!is_token(LT_KEYWORD_FN)) {
                return nullptr;
            }

            consume();

            _log(debug, "Found \'%s\'", lexer_token_hr_names[m_current.type]);

            return nullptr;

            switch (m_current.type) {
                // Anonymous without arg-list nor return-type
                case LT_COLON: {
                    
                } break;

                // Named
                case LT_IDENT: {

                } break;

                // Anonymous with arg-list 
                case LT_OPENING_PARENT: {

                } break;

                // Anonymous with return-type but no arg-list
                case LT_ARROW: {

                } break;
            }
        }

        expression_t* parse_expression_impl() {
            expression_t* expr;
        
            switch (m_current.type) {
                case LT_KEYWORD_FN: {
                    expr = parse_function_def();
                } break;

                // Could be many things...
                case LT_IDENT: {
                    if (!m_ts.exists(m_current.text)) {
                        // Name ref
                    } else {
                        // Definition
                        hs_type_t* type = parse_type();
                    }
                } break;

                case LT_KEYWORD_MUT: {
                    hs_type_t* type = parse_type();
                } break;

                default: {
                    ERROR(fmt("Unhandled token \"" ESCAPE(37;1) "%s" ESCAPE(0) "\"", m_current.text.c_str()));

                    expr = nullptr;
                };
            }

            return expr;
        }

        inline bool is_type_modifier() {
            return (m_current.type == LT_KEYWORD_MUT    ) ||
                   (m_current.type == LT_KEYWORD_STATIC ) ||
                   (m_current.type == LT_KEYWORD_CONST  );
        }

        hs_type_t* parse_type() {
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

            _log(debug, "signature=%s", signature.c_str());

            m_ts.add_type(signature, type);
        }

        // -- Support functions --

        expression_t* parse_rhs(expression_t* lhs) {
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

        expression_t* parse_expression() {
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

        bool parse() {
            expression_t* lhs;
            expression_t* op;

            m_current = m_input->get();

            while (m_current.type != LT_NONE) {
                expression_t* expr = parse_expression();

                m_output.source.push_back(expr);
            }

            return true;
        }
    };
}

#undef ERROR