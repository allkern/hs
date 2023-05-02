#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace hs {
    struct expression_block_t : public expression_t {
        std::vector <expression_t*> block;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(expression-block: type=\"" << type << "\", assignee=" << assignee->print(0) << ", value=" << value->print(0) << ")"; 
#else
            ss << "(block:\n";

            for (expression_t* expr : block) {
                ss << expr->print(hierarchy + 1) << "\n";
            }
#endif
            ss << std::string(hierarchy, ' ');
            ss << ")";

            return ss.str();
        }

        expression_type_t get_expr_type() override {
            return EX_EXPRESSION_BLOCK;
        }
        
        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };
}