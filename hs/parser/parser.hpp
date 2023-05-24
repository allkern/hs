#pragma once

#include "../lexer/token.hpp"   
#include "../lexer/lexer.hpp"

#include "../stream.hpp"
#include "../error.hpp"

#include "expression.hpp"
#include "output.hpp"

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
        "none",
        "array-access",
        "raw-memory-access",
        "assignment",
        "binary-op",
        "unary-op",
        "comp-op",
        "expression-block",
        "function-call",
        "function-def",
        "invoke",
        "name-ref",
        "numeric-literal",
        "string-literal",
        "type",
        "variable-def",
        "asm-block",
        "while-loop",
        "array",
        "blob",
        "if-else",
        "return"
    };

    class parser_t {
        stream_t <lexer_token_t>*   m_input;
        parser_output_t             m_output;
        error_logger_t*             m_logger;
        lexer_token_t               m_current;
        type_system_t               m_ts;

        int m_anonymous_functions = 0;

    public:
        error_logger_t* get_logger();
        std::string get_anonymous_function_name();
        void init(stream_t <lexer_token_t>*, error_logger_t*);
        parser_output_t* get_output();
        bool is_token(lexer_token_type_t);
        bool expect_token(lexer_token_type_t);
        expression_t* expect_expr(expression_type_t, expression_t*);
        expression_t* try_expr(expression_type_t, expression_t*);
        void consume();
        inline lexer_token_t current();
        expression_t* parse_expression_impl();
        inline bool is_type_modifier();
        function_type_t* parse_function_type();
        hs_type_t* parse_type();
        definition_t parse_definition();
        type_system_t* type_system();
        bool is_type();
        void init_expr(expression_t*);
        
        // -- Support functions --
        expression_t* parse_rhs(expression_t* lhs);
        expression_t* parse_expression();
        bool parse();
    };

}

#undef ERROR