#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct asm_block_t : public expression_t {
        std::string assembly;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(asm-block: asm=" << assembly << ")"; 
#else
            ss << "(asm " << assembly << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_ASM_BLOCK;
        }
    };
}