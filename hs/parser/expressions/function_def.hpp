#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"
#include "../parser.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace hs {
    struct function_def_t : public expression_t {
        expression_t*               body = nullptr;
        hs_type_t*                  return_type = nullptr;
        std::vector <definition_t>  args;
        std::string                 name;
        hs_type_t*                  fty = nullptr;

        std::string print(int hierarchy) override {
            return "function_def_t to-do";
        }

        expression_type_t get_expr_type() override {
            return EX_FUNCTION_DEF;
        }

        hs_type_t* get_hs_type() override {
            return fty;
        }
    };
}