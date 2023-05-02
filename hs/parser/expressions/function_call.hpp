#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace hs {
    struct function_call_t : public expression_t {
        expression_t* addr = nullptr;
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

        expression_type_t get_expr_type() override {
            return EX_FUNCTION_CALL;
        }
        
        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };
}