#pragma once

#include <string>
#include <sstream>
#include <unordered_map>

namespace hs {
    std::unordered_map <std::string, size_t> types = {
        { "none", 0 },
        { "u8"  , 1 },
        { "u16" , 2 },
        { "u32" , 4 },
        { "i8"  , 1 },
        { "i16" , 2 },
        { "i32" , 4 }
    };

    std::unordered_map <std::string, std::string> type_aliases = {
        { "void" , "none" },
        { "byte" , "u8"   },
        { "uchar", "u8"   },
        { "char" , "i8"   },
        { "short", "u16"  },
        { "int"  , "i32"  },
        { "long" , "u32"  }
    };

    struct type_t : public expression_t {
        std::string type;

        std::string print(int hierarchy) override {
            std::ostringstream ss;

            ss << std::string(hierarchy, ' ');

#ifndef HS_AST_PRINT_FORMAT_LISP
            ss << "(type-expr: type=" << type << ")"; 
#else
            ss << "(type " << type << ")"; 
#endif
            return ss.str();
        }

        expression_type_t get_type() override {
            return EX_TYPE;
        }
    };
}