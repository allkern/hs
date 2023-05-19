#include "parser.hpp"

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "parser", \
        msg, \
        m_current.line, m_current.offset, m_current.text.size() \
    ); \

namespace hs {
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

    expression_t* parser_t::expect_expr(parser_expression_t pt, expression_t* arg) {
        expression_parser_t p = m_expression_parsers[pt];

        assert(p);

        expression_t* expr = p(this, arg);

        if (!expr) {
            ERROR(fmt("Expected " ESCAPE(37) "\'%s\'", parser_expression_hr_names[pt]));

            return nullptr;
        }

        return expr;
    }

    expression_t* parser_t::try_expr(parser_expression_t pt, expression_t* arg) {
        expression_parser_t p = m_expression_parsers[pt];

        assert(p);

        size_t pos = m_input->tellg();

        expression_t* expr = p(this, arg);

        if (!expr) {
            m_input->seekg(pos);

            return nullptr;
        }

        return expr;
    }

    lexer_token_t parser_t::consume() {
        m_current = m_input->get();
    }

    expression_t* parser_t::parse_expression_impl() {
        expression_t* expr;
    
        switch (m_current.type) {
            case LT_KEYWORD_FN: {
                expr = parse_function_def();
            } break;

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

    inline bool parser_t::is_type_modifier() {
        return  (m_current.type == LT_KEYWORD_MUT   ) ||
                (m_current.type == LT_KEYWORD_STATIC) ||
                (m_current.type == LT_KEYWORD_CONST );
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
        expression_t* lhs;
        expression_t* op;

        m_current = m_input->get();

        while (m_current.type != LT_NONE) {
            expression_t* expr = parse_expression();

            m_output.source.push_back(expr);
        }

        return true;
    }
}

#undef ERROR