#pragma once

#include <string>

#define HS_AST_PRINT_FORMAT_LISP
#define HS_AST_PRINT_INDENT_SIZE 2

namespace hs {
    enum eval_type_t {
        ET_NULL,
        ET_NUMERIC
    };

    class eval_t {
        eval_type_t type;

        uint64_t value;
    };

    class expression_t {
    public:
        virtual std::string print(int hierarchy) { return "<undefined>"; };
        virtual eval_t eval() { return eval_t(); };
    };
}