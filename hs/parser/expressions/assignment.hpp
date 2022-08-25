#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct assignment_t : public expression_t {
        std::string type;
        expression_t* assignee;
        expression_t* value;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(assignment: type=\"" << type << "\", assignee=" << assignee->print(0) << ", value=" << value->print(0) << ")"; 
#else
            ss << "(" << type << " " << assignee->print(0) << " " << value->print(0) << ")"; 
#endif
            return ss.str();
        }
    };
}