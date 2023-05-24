#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct variable_def_t : public expression_t {
        hs_type_t* type;
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

        expression_type_t get_expr_type() override {
            return EX_VARIABLE_DEF;
        }
        
        hs_type_t* get_hs_type() override {
            // Can't optimize compiler memory usage here
            return new pointer_type_t(type);
        }
    };
}