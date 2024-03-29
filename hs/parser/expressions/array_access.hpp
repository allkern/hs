#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct array_access_t : public expression_t {
        // To-do: IR Unimplemented
        // Implement indexed array access

        expression_t* type_or_name = nullptr;
        expression_t* addr = nullptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(memory-access: type=" << type << ", addr=" << init->print(0) << ")"; 
#else
            ss << "(access " << type_or_name->print(0) << " " << addr->print(0) << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_ARRAY_ACCESS;
        }
    };
}