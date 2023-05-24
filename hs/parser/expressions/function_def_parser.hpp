#pragma once

#include "../expression.hpp"
#include "../type_system.hpp"
#include "../parser.hpp"

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
    // fn: expr
    // fn name: expr
    // fn (arg-list): expr
    // fn -> type: expr
    // fn name(arg-list): expr
    // fn name -> type: expr
    // fn (arg-list) -> type: expr
    expression_t* parse_function_def(parser_t* parser, expression_t* prev) {
        if (!parser->is_token(LT_KEYWORD_FN)) {
            return nullptr;
        }

        function_def_t* fd = new function_def_t;

        parser->init_expr(fd);

        // Consume "fn"
        parser->consume();

        while (parser->current().type != LT_COLON) {
            // Named
            if (parser->current().type == LT_IDENT) {
                if (parser->is_type()) {
                    // To-do error: Expected identifier

                    return nullptr;
                }

                fd->name = parser->current().text;

                parser->consume();

                continue;
            }

            // Anonymous with arg-list 
            if (parser->current().type == LT_OPENING_PARENT) {
                parser->consume();

                while (parser->current().type != LT_CLOSING_PARENT) {
                    fd->args.push_back(parser->parse_definition());

                    switch (parser->current().type) {
                        case LT_COMMA: {
                            parser->consume();
                        } break;

                        case LT_CLOSING_PARENT: continue;

                        default: {
                            // Error: unexpected token
                        } break;
                    }

                    continue;
                }

                parser->consume();
                

                continue;
            }

            // Anonymous with return-type but no arg-list
            if (parser->current().type == LT_ARROW) {
                parser->consume();

                fd->return_type = parser->parse_type();

                if (!fd->return_type) {
                    ERROR("Identifier does not name a type");
                }

                continue;
            }
        }

        if (!fd->name.size()) {
            fd->name = parser->get_anonymous_function_name();
        }

        parser->consume();

        fd->body = parser->parse_expression();

        if (!fd->return_type) {
            fd->return_type = fd->body->get_hs_type();
        } else {
            // Check types
            if (!parser->type_system()->type_eq(fd->return_type, fd->body->get_hs_type())) {
                // Warning: Body type != specified return type
                _log(warning, "Body return type is not equal to specified return type");
            }
        }

        function_type_t* fty = new function_type_t;

        fty->args           = fd->args;
        fty->return_type    = fd->return_type;
        fty->mut            = false;
        fty->is_static      = false;

        std::string signature = get_signature(fty);

        fty->signature = signature;

        if (parser->type_system()->exists(signature)) {
            delete fty;

            fd->fty = parser->type_system()->get_type(signature);
        } else {
            parser->type_system()->add_type(signature, fty);

            fd->fty = parser->type_system()->get_type(signature);
        }

        _log(debug, "function-def: name=%s, type=%s", fd->name.c_str(), fd->fty->signature.c_str());

        return fd;
    }
}

#undef ERROR