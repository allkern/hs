#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct string_literal_t : public expression_t {
        std::string str;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(string-literal: value=" << value << ")";
#else
            ss << "\"" << str << "\"";
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_STRING_LITERAL;
        }
    };
}