#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct array_t : public expression_t {
        type_t type;
        unsigned int size = 0;
        std::vector <expression_t*> values;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(array: type=" << type.type << ", size=" << size << ", {";

            for (expression_t* expr : values)
                ss << expr->print(0) << ", ";

            ss << "}" << ")";
#else
            ss << "(arr type=" << type.type << ", size=" << size << ", {";

            for (expression_t* expr : values)
                ss << expr->print(0) << ", ";

            ss << "}" << ")";
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_ARRAY;
        }
    };
}