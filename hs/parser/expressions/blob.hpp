#pragma once

#include "../expression.hpp"

#include <string>
#include <sstream>

namespace hs {
    struct blob_t : public expression_t {
        unsigned int size = 0;

        std::string file;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

            ss << "(blob " << file << ")";

            return ss.str();
        }

        expression_type_t get_expr_type() override {
            return EX_BLOB;
        }
        
        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };
}