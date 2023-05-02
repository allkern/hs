#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct numeric_literal_t : public expression_t {
        uint64_t value;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(numeric-literal: value=" << value << ")";
#else
            ss << value;
#endif
            return ss.str();
        }

        expression_type_t get_expr_type() override {
            return EX_NUMERIC_LITERAL;
        }
        
        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };
}