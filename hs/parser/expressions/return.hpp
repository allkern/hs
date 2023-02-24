#pragma once

#include "../expression.hpp"
#include "type.hpp"

namespace hs {
    struct return_expr_t : public expression_t {
        expression_t* value;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(return: value=" << value->print(0) << ")";
#else
            ss << "(ret: " << value->print(0) << ")";
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_RETURN;
        }
    };
}