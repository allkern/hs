#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"
#include "../parser.hpp"

#include "numeric_literal.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace hs {
    expression_t* parse_numeric_literal(parser_t* parser, expression_t* prev) {
        if (!parser->is_token(LT_LITERAL_NUMERIC)) {
            return nullptr;
        }

        numeric_literal_t* num = new numeric_literal_t;

        parser->init_expr(num);

        if (!std::isdigit(parser->current().text[0])) {
            // This is a char literal
            
            num->value = parser->current().text[0];
        } else {
            num->value = std::stoull(parser->current().text, nullptr, 0);
        }

        parser->consume();

        return num;
    }
}