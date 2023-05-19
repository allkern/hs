#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace hs {
    struct function_def_t : public expression_t {
        expression_t*               body = nullptr;
        hs_type_t*                  return_type;
        std::vector <definition_t>  args;
        std::string                 name;

        std::string print(int hierarchy) override {
            return "function_def_t to-do";
        }

        expression_type_t get_expr_type() override {
            return EX_FUNCTION_DEF;
        }

        hs_type_t* get_hs_type() override {
            return nullptr; /* To-do */
        }
    };

    // fn: expr
    // fn name: expr
    // fn (arg-list): expr
    // fn -> type: expr
    // fn name(arg-list): expr
    // fn name -> type: expr
    // fn (arg-list) -> type: expr
    expression_t* parse_function_def() {
        return nullptr;
    }

    // expression_t* parse_function_def(parser_t* parser) {
    //     if (!parser->is_token(LT_KEYWORD_FN)) {
    //         return nullptr;
    //     }

    //     parser->consume();

    //     return nullptr;

    //     switch (m_current.type) {
    //         // Anonymous without arg-list nor return-type
    //         case LT_COLON: {
                
    //         } break;

    //         // Named
    //         case LT_IDENT: {

    //         } break;

    //         // Anonymous with arg-list 
    //         case LT_OPENING_PARENT: {

    //         } break;

    //         // Anonymous with return-type but no arg-list
    //         case LT_ARROW: {

    //         } break;
    //     }
    // }
}