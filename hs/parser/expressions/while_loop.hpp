#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct while_loop_t : public expression_t {
        expression_t* condition;
        expression_t* body;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(while-loop: condition=" << condition->print(0) << ")"; 
#else
            ss << "(while " << condition->print(0) << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_WHILE_LOOP;
        }
    };
}