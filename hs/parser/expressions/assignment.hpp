#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct assignment_t : public expression_t {
        std::string op;
        expression_t* assignee = nullptr;
        expression_t* value = nullptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(assignment: op=\"" << op << "\", assignee=" << assignee->print(0) << ", value=" << value->print(0) << ")"; 
#else
            ss << "(" << op << " " << assignee->print(0) << " " << value->print(0) << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_ASSIGNMENT;
        }
    };
}