#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace hs {
    struct unary_op_t : public expression_t {
        bool post = false;
        std::string op;
        expression_t* operand = nullptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(unary-op: " << op << operand->print(0) << ")";
#else
            ss << "(" << op << " " << operand->print(0) << ")";
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_UNARY_OP;
        }
    };
}