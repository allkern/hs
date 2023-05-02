#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

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

        expression_type_t get_expr_type() override {
            return EX_NAME_REF;
        }
        
        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };
}