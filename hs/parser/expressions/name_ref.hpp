#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct name_ref_t : public expression_t {
        std::string name;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(name-ref: name=" << name << ")"; 
#else
            ss << "(ref " << name << ")"; 
#endif
            return ss.str();
        }
    };
}