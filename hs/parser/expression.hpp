#pragma once

#include <string>

#define HS_AST_PRINT_FORMAT_LISP
#define HS_AST_PRINT_INDENT_SIZE 2

namespace hs {
    enum eval_type_t {
        ET_NULL,
        ET_NUMERIC
    };

    enum expression_type_t {
        EX_NONE,
        EX_ARRAY_ACCESS,
        EX_ASSIGNMENT,
        EX_BINARY_OP,
        EX_EXPRESSION_BLOCK,
        EX_FUNCTION_CALL,
        EX_FUNCTION_DEF,
        EX_INVOKE,
        EX_NAME_REF,
        EX_NUMERIC_LITERAL,
        EX_TYPE,
        EX_VARIABLE_DEF
    };

    class eval_t {
        eval_type_t type;

        uint64_t value;
    };

    class expression_t {
    public:
        int line, offset, len;

        virtual std::string print(int hierarchy) { return "<undefined>"; };
        virtual eval_t eval() { return eval_t(); };
        virtual expression_type_t get_type() { return EX_NONE; };
    };
}