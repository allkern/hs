#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace hs {
    struct invoke_expr_t : public expression_t {
        expression_t* ptr;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, '\t');
            ss << "(invoke-expr:\n" << std::hex << std::setfill('0') << std::setw(8) << ptr->print(hierarchy + 1);

            return ss.str();
        }
    };
}