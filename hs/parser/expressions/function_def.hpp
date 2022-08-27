#pragma once

#include "../expression.hpp"
#include "type.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace hs {
    struct function_arg_t {
        std::string type;
        std::string name;
    };

    struct function_def_t : public expression_t {
        std::string name;
        expression_t* body;
        std::string type;
        std::vector <function_arg_t> args;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy * HS_AST_PRINT_INDENT_SIZE, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(function-def: name=" << name << ", "
               << "type=" << type << ", "
               << "args={";

            for (unsigned int i = 0; i < args.size(); i++) {
                ss << "(arg: type=" << args[i].type << ", name=" << args[i].name << ")";

                if (i < (args.size() - 1))
                    ss << ", ";
            }

            ss << "}, "
               << "\n" << ;
#else
            ss << "(fn " << name << ": \n" << (body ? body->print(hierarchy + 1) : "<no_body>") << "\n" << std::string(hierarchy, ' ') << ')';
#endif
            return ss.str();
        }

        expression_type_t type() override {
            return EX_FUNCTION_DEF;
        }
    };
}