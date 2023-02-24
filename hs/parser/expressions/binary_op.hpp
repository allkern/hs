#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace hs {
    enum binary_operator_t : char {
        BOP_ADD = '+',
        BOP_SUB = '-',
        BOP_MUL = '*',
        BOP_DIV = '/'
    };

    struct binary_op_t : public expression_t {
        std::string op;
        expression_t* lhs = nullptr;
        expression_t* rhs = nullptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(binary-op: " << lhs->print(0) << ", " << rhs->print(0) << ")";
#else
            ss << "(" << op << " " << lhs->print(0) << " " << rhs->print(0) << ")";
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_BINARY_OP;
        }
    };
}