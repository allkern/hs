#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"
#include "../parser.hpp"

#include "../expressions/variable_def.hpp"

#include <string>
#include <vector>
#include <sstream>

#define ERROR(msg) \
    if (parser->get_logger()) parser->get_logger()->print_error( \
        "parser", \
        msg, \
        parser->current().line, parser->current().offset, parser->current().text.size() \
    ); \
    std::exit(1);

namespace hs {
    expression_t* parse_variable_def(parser_t* parser, expression_t* prev) {
        if (!parser->is_type()) {
            return nullptr;
        }

        variable_def_t* vd = new variable_def_t;

        parser->init_expr(vd);

        vd->type = parser->parse_type();

        if (parser->current().type != LT_IDENT) {
            return vd;

            ERROR("Expected variable name after type");
        }

        vd->name = parser->current().text;

        parser->consume();

        return vd;
    }
}

#undef ERROR