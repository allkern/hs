#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct variable_def_t : public expression_t {
        std::string type;
        std::string name;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(variable-def: name=" << name << ", type=" << type << ")"; 
#else
            ss << "(def " << type << " " << name << ")"; 
#endif
            return ss.str();
        }
    };
}