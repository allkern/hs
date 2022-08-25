#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace hs {
    struct function_call_t : public expression_t {
        expression_t* addr;
        std::vector <expression_t*> args;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(function-call: addr=" << addr->print(0) << ", args={";
            
            for (unsigned int i = 0; i < args.size(); i++) {
                ss << args[i]->print(0);

                if (i < (args.size() - 1))
                    ss << ", ";
            }

            ss << "})";
#else
            ss << "(call " << addr->print(0) << " ";
            
            for (unsigned int i = 0; i < args.size(); i++) {
                ss << args[i]->print(0);

                if (i < (args.size() - 1))
                    ss << " ";
            }

            ss << ")";
#endif
            return ss.str();
        }
    };
}