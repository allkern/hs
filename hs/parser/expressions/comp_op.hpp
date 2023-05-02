#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace hs {
    struct comp_op_t : public expression_t {
        std::string op;
        expression_t* lhs = nullptr;
        expression_t* rhs = nullptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(comp-op: " << lhs->print(0) << ", " << rhs->print(0) << ")";
#else
            ss << "(" << op << " " << lhs->print(0) << " " << rhs->print(0) << ")";
#endif
            return ss.str();
        }

        expression_type_t get_expr_type() override {
            return EX_COMP_OP;
        }
        
        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };
}