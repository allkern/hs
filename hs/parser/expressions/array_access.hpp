#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct array_access_t : public expression_t {
        expression_t* name = nullptr;
        expression_t* addr = nullptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(memory-access: type=" << type << ", addr=" << init->print(0) << ")"; 
#else
            ss << "(access " << name->print(0) << " " << addr->print(0) << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_expr_type() override {
            return EX_ARRAY_ACCESS;
        }

        hs_type_t* get_hs_type() override {
            return name->get_hs_type();
        }
    };
}