#pragma once

#include "expression.hpp"
#include "expressions/function_def.hpp"
#include "expressions/variable_def.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace hs {
    struct variable_t {
        std::string scope;

        variable_def_t* def;
    };

    struct parser_output_t {
        std::vector <function_def_t*> functions;
        std::vector <variable_t> variables;
        std::vector <expression_t*> source;
    };
}