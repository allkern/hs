#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct if_else_t : public expression_t {
        expression_t* cond;
        expression_t* if_expr;
        expression_t* else_expr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(if-else: if=" << if_expr->print(0) << ", else=" << else_expr->print(0) << ")"; 
#else
            ss << "(if-else " << if_expr->print(0) << " " << else_expr->print(0) << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_WHILE_LOOP;
        }
    };
}