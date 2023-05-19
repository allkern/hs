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
        void init(stream_t <lexer_token_t>*, error_logger_t*);
        parser_output_t* get_output();
        bool is_token(lexer_token_type_t);
        bool expect_token(lexer_token_type_t);
        expression_t* expect_expr(parser_expression_t, expression_t*);
        expression_t* try_expr(parser_expression_t, expression_t*);
        lexer_token_t consume();
        expression_t* parse_expression_impl();
        inline bool is_type_modifier();
        hs_type_t* parse_type();
        
        // -- Support functions --

        expression_t* parse_rhs(expression_t* lhs);
        expression_t* parse_expression();
        bool parse();
    };

    typedef expression_t* (*expression_parser_t)(parser_t*, expression_t*);

    std::unordered_map <parser_expression_t, expression_parser_t> m_expression_parsers; // = {
    //     { PT_FUNCTION_DEF, parse_function_def }
    // };
}

#undef ERROR