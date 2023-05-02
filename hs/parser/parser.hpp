#pragma once

#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"

#include "../stream.hpp"
#include "../error.hpp"

#include "expression.hpp"
#include "output.hpp"

#include "expressions/expression_block.hpp"
#include "expressions/numeric_literal.hpp"
#include "expressions/string_literal.hpp"
#include "expressions/function_call.hpp"
#include "expressions/array_access.hpp"
#include "expressions/variable_def.hpp"
#include "expressions/function_def.hpp"
#include "expressions/assignment.hpp"
#include "expressions/while_loop.hpp"
#include "expressions/asm_block.hpp"
#include "expressions/binary_op.hpp"
#include "expressions/name_ref.hpp"
#include "expressions/comp_op.hpp"
#include "expressions/if_else.hpp"
#include "expressions/return.hpp"
#include "expressions/invoke.hpp"
#include "expressions/array.hpp"
#include "expressions/blob.hpp"

#include "type_system.hpp"

#include <cassert>

#define ERROR(msg) \
    if (m_logger) m_logger->print_error( \
        "parser", \
        msg, \
        m_current.line, m_current.offset, m_current.text.size() \
    ); \

namespace hs {
    static const char* parser_expression_hr_names[] = {
        "function-def"
    };

    enum parser_expression_t : int {
        PT_FUNCTION_DEF
    };

    class parser_t {
        stream_t <lexer_token_t>* m_input;
        error_logger_t* m_logger;

        lexer_token_t m_current;
        parser_output_t m_output;

        type_system_t ts;

        int m_anonymous_functions = 0;

        std::string get_anonymous_function_name() {
            std::string name = "<anonymous_";

            name += std::to_string(m_anonymous_functions++);
            name += ">";

            return name;
        }
    
    public:
        void init(stream_t <lexer_token_t>* input, error_logger_t* logger) {
            m_input = input;
            m_logger = logger;

            ts.init();
        }

        typedef expression_t* (parser_t::*expression_parser_t)(expression_t*);

<<<<<<< Updated upstream
        expression_t* parse_expression_impl();
        expression_t* parse_expression();

        expression_t* parse_function_def() {
            if (m_current.type != LT_KEYWORD_FN) {
                assert(false); // ??
            }

            function_def_t* def = new function_def_t;

            def->line = m_current.line;
            def->offset = m_current.offset;
            def->len = m_current.text.size();

            m_current = m_input->get();

            switch (m_current.type) {
                case LT_ARROW: {
                    def->name = get_anonymous_function_name();

                    goto parse_function_type;
                } break;
                
                case LT_COLON: {
                    def->name = get_anonymous_function_name();
                    def->type = "<any>";

                    goto parse_function_body;
                } break;

                case LT_OPENING_PARENT: {
                    def->name = get_anonymous_function_name();

                    goto parse_function_args;
                } break;

                default: break;
            }

            if (m_current.type != LT_IDENT) {
                ERROR("Expected " ESCAPE(37;1) "function name" ESCAPE(0) " after " ESCAPE(37;1) "fn" ESCAPE(0) " keyword");
            }

            def->name = m_current.text;

            parse_remaining_prototype:

            m_current = m_input->get();

            switch (m_current.type) {
                case LT_OPENING_PARENT: {
                    // Parse arguments
                    parse_function_args:

                    function_arg_t arg;

                    parse_arg:

                    m_current = m_input->get();

                    switch (m_current.type) {
                        case LT_IDENT: goto do_parse_arg;
                        case LT_CLOSING_PARENT: goto parse_remaining_prototype;
                        default: {
                            ERROR("Expected " ESCAPE(37;1) "argument name" ESCAPE(0));
                        } break;
                    }

                    do_parse_arg:

                    arg.name = m_current.text;

                    m_current = m_input->get();

                    if (m_current.type != LT_COLON) {
                        ERROR("Expected \'" ESCAPE(37;1) ":" ESCAPE(0) "\' after name");
                    }

                    m_current = m_input->get();

                    if (m_current.type != LT_IDENT) {
                        ERROR("Expected " ESCAPE(37;1) "argument type" ESCAPE(0));
                    }

                    if (!is_type(m_current.text)) {
                        ERROR(fmt("Identifier \"" ESCAPE(37;1) "%s" ESCAPE(0) "\" does not name a type", m_current.text.c_str()));
                    }

                    arg.type = m_current.text;

                    m_current = m_input->get();

                    switch (m_current.type) {
                        case LT_COMMA: {
                            def->args.push_back(arg);

                            goto parse_arg;
                        } break;

                        case LT_CLOSING_PARENT: {
                            def->args.push_back(arg);

                            goto parse_remaining_prototype;
                        } break;

                        default: {
                            ERROR("Expected \'" ESCAPE(37;1) "," ESCAPE(0) "\' or \'" ESCAPE(37;1) ")" ESCAPE(0) "\'");
                        } break;
                    }
                } break;

                case LT_ARROW: {
                    parse_function_type:

                    // Parse function type
                    m_current = m_input->get();

                    if (m_current.type != LT_IDENT) {
                        ERROR("Expected " ESCAPE(37;1) "type" ESCAPE(0) " after " ESCAPE(37;1) "->" ESCAPE(0) " on function definition");
                    }

                    if (!is_type(m_current.text)) {
                        ERROR(fmt("Identifier \"" ESCAPE(37;1) "%s" ESCAPE(0) "\" does not name a type", m_current.text.c_str()));
                    }

                    def->type = m_current.text;

                    goto parse_remaining_prototype;
                } break;

                case LT_COLON: {
                    parse_function_body:

                    m_current = m_input->get();

                    def->body = parse_expression();

                    if (!def->body) return nullptr;
                } break;

                default: {
                    ERROR("Expected " ESCAPE(37;1) "parameters" ESCAPE(0) " or " ESCAPE(37;1) "type" ESCAPE(0) " after " ESCAPE(37;1) "function name" ESCAPE(0));
                } break;
            }

            return def;
        }

        expression_t* parse_numeric_literal() {
            if (m_current.type != LT_LITERAL_NUMERIC) {
                assert(false); // ??
            }

            numeric_literal_t* num = new numeric_literal_t;

            num->line = m_current.line;
            num->offset = m_current.offset;
            num->len = m_current.text.size();

            if (!std::isdigit(m_current.text[0])) {
                // This is a char literal
                
                num->value = m_current.text[0];
            } else {
                num->value = std::stoull(m_current.text, nullptr, 0);
            }

            m_current = m_input->get();

            return num;
        }

        expression_t* parse_string_literal() {
            if (m_current.type != LT_LITERAL_STRING) {
                assert(false); // ??
            }

            string_literal_t* str = new string_literal_t;

            str->line = m_current.line;
            str->offset = m_current.offset;
            str->len = m_current.text.size();

            str->str = m_current.text;

            m_current = m_input->get();

            return str;
        }

        expression_t* parse_invoke() {
            if (m_current.type != LT_KEYWORD_INVOKE) {
                assert(false); // ??
            }

            invoke_expr_t* invoke = new invoke_expr_t;

            invoke->line = m_current.line;
            invoke->offset = m_current.offset;
            invoke->len = m_current.text.size();

            m_current = m_input->get();

            invoke->ptr = parse_expression();

            return invoke;
        }

        expression_t* parse_binary_op(expression_t* lhs) {
            binary_op_t* bop = new binary_op_t;

            bop->line = m_current.line;
            bop->offset = m_current.offset;
            bop->len = m_current.text.size();

            bop->op = m_current.text;

            m_current = m_input->get();
            
            bop->lhs = lhs;
            bop->rhs = parse_expression();

            return bop;
        }

        expression_t* parse_comp_op(expression_t* lhs) {
            comp_op_t* cop = new comp_op_t;

            cop->line = m_current.line;
            cop->offset = m_current.offset;
            cop->len = m_current.text.size();

            cop->op = m_current.text;

            m_current = m_input->get();
            
            cop->lhs = lhs;
            cop->rhs = parse_expression();

            return cop;
        }

        expression_t* parse_variable_def(std::string type) {
            variable_def_t* var = new variable_def_t;

            var->line = m_current.line;
            var->offset = m_current.offset;
            var->len = m_current.text.size();

            var->type = type;
            var->name = m_current.text;

            m_current = m_input->get();

            return var;
        }

        expression_t* parse_blob() {
            blob_t* blob = new blob_t;

            if (m_current.type != LT_KEYWORD_BLOB) {
                assert(false); // ??
            }

            blob->line = m_current.line;
            blob->offset = m_current.offset;
            blob->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_LITERAL_STRING) {
                ERROR("Expected string literal after blob");
            }

            blob->file = m_current.text;

            m_current = m_input->get();

            return blob;
        }

        expression_t* parse_array() {
            array_t* arr = new array_t;

            if (m_current.type != LT_KEYWORD_ARRAY) {
                assert(false); // ??
            }

            arr->line = m_current.line;
            arr->offset = m_current.offset;
            arr->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_OPENING_BRACKET) {
                ERROR("Expected [ after array");
            }

            m_current = m_input->get();

            if (m_current.type != LT_IDENT) {
                ERROR("Expected type after [");
            }

            if (!is_type(m_current.text)) {
                ERROR(fmt("Identifier \"" ESCAPE(37;1) "%s" ESCAPE(0) "\" does not name a type", m_current.text.c_str()));
            }

            arr->type = type_t();
            arr->type.type = m_current.text;

            m_current = m_input->get();

            if (m_current.type == LT_COMMA) {
                m_current = m_input->get();

                if (m_current.type != LT_LITERAL_NUMERIC) {
                    ERROR("Expected numeric literal after [");
                }

                if (!std::isdigit(m_current.text[0])) {
                    // This is a char literal
                    arr->size = m_current.text[0];
                } else {
                    arr->size = std::stoull(m_current.text, nullptr, 0);
                }

                m_current = m_input->get();
            }

            if (m_current.type != LT_CLOSING_BRACKET) {
                ERROR("Expected ] after array type or size");
            }

            m_current = m_input->get();

            if (m_current.type == LT_OPENING_BRACE) {
                expression_t* lhs;
                expression_t* op;

                m_current = m_input->get();

                unsigned int real_size = 0;

                while (m_current.type != LT_CLOSING_BRACE) {
                    lhs = parse_expression();

                    do {
                        op = parse_rightside_operation(lhs);

                        if (op) lhs = op;
                    } while (op);

                    arr->values.push_back(lhs);
                    real_size++;

                    if (m_current.type == LT_CLOSING_BRACE) break;

                    if (m_current.type != LT_COMMA) {
                        ERROR("Expressions on arrays must be separated by commas");
                    }

                    m_current = m_input->get();
                }

                if (m_current.type == LT_CLOSING_BRACE) {
                    if ((real_size != arr->size) && (arr->size != 0)) {
                        if (m_logger) m_logger->print_warning(
                            "parser",
                            "Array size doesn't match declared size",
                            m_current.line, m_current.offset, m_current.text.size(), true
                        );
                    }

                    arr->size = real_size;
                }

                m_current = m_input->get();
            }

            if (!arr->size) {
                ERROR("Cannot define zero-sized arrays");
            }

            return arr;
        }

        expression_t* parse_name_ref(std::string name) {
            name_ref_t* ref = new name_ref_t;

            ref->line = m_current.line;
            ref->offset = m_current.offset;
            ref->len = m_current.text.size();
            
            ref->name = name;

            return ref;
        }

        expression_t* parse_if_else() {
            if (m_current.type != LT_KEYWORD_IF) {
                assert(false); // ??
            }

            if_else_t* ifl = new if_else_t;

            ifl->line = m_current.line;
            ifl->offset = m_current.offset;
            ifl->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_OPENING_PARENT) {
                ERROR("Expected opening parenthesis after if");
            }

            m_current = m_input->get();

            ifl->cond = parse_expression();

            if (m_current.type != LT_CLOSING_PARENT) {
                ERROR("Expected closing parenthesis after if condition");
            }

            m_current = m_input->get();

            if (m_current.type != LT_COLON) {
                ERROR("Expected colon after closing parenthesis");
            }

            m_current = m_input->get();

            ifl->if_expr = parse_expression();

            if (m_current.type == LT_KEYWORD_ELSE) {
                m_current = m_input->get();

                if (m_current.type != LT_COLON) {
                    ERROR("Expected colon after else");
                }

                m_current = m_input->get();

                ifl->else_expr = parse_expression();
            }

            return ifl;
        }

        expression_t* parse_while_loop() {
            if (m_current.type != LT_KEYWORD_WHILE) {
                assert(false); // ??
            }

            while_loop_t* whl = new while_loop_t;

            whl->line = m_current.line;
            whl->offset = m_current.offset;
            whl->len = m_current.text.size();

            m_current = m_input->get();

            if (m_current.type != LT_OPENING_PARENT) {
                ERROR("Expected opening parenthesis after while loop declaration");
            }

            m_current = m_input->get();

            whl->condition = parse_expression();

            if (m_current.type != LT_CLOSING_PARENT) {
                ERROR("Expected closing parenthesis after while loop condition");
            }

            m_current = m_input->get();

            if (m_current.type != LT_COLON) {
                ERROR("Expected colon after closing parenthesis");
            }

            m_current = m_input->get();

            whl->body = parse_expression();

            return whl;
        }

        expression_t* parse_return() {
            assert(m_current.type == LT_KEYWORD_RETURN);

            return_expr_t* ret = new return_expr_t;

            ret->line = m_current.line;
            ret->offset = m_current.offset;
            ret->len = m_current.text.size();

            m_current = m_input->get();

            ret->value = parse_expression();

            return ret;
        }

        expression_t* parse_array_access(expression_t* lhs) {
            array_access_t* access = new array_access_t;

            access->line = m_current.line;
            access->offset = m_current.offset;
            access->len = m_current.text.size();

            access->type_or_name = lhs;

            access->addr = parse_expression();

            if (m_current.type != LT_CLOSING_BRACKET) {
                ERROR("Expected closing bracket on array access expression");

                return nullptr;
            }

            m_current = m_input->get();

            return access;
        }

        expression_t* parse_function_call(expression_t* addr) {
            function_call_t* call = new function_call_t;

            call->line = m_current.line;
            call->offset = m_current.offset;
            call->len = m_current.text.size();

            call->addr = addr;

            if (m_current.type == LT_CLOSING_PARENT) {
                m_current = m_input->get();

                return call;
            }

            while (m_current.type != LT_CLOSING_PARENT) {
                call->args.push_back(parse_expression());

                if (m_current.type == LT_CLOSING_PARENT) {
                    m_current = m_input->get();

                    break;
                }

                if (m_current.type != LT_COMMA) {
                    ERROR("Expected comma after call argument");
                }

                m_current = m_input->get();
            }

            return call;
        }

        expression_t* parse_assignment(expression_t* lhs) {
            assignment_t* assign = new assignment_t;

            assign->line = m_current.line;
            assign->offset = m_current.offset;
            assign->len = m_current.text.size();

            assign->assignee = lhs;
            assign->op = m_current.text;

            m_current = m_input->get();

            assign->value = parse_expression();

            return assign; 
        }

        expression_t* parse_asm_block() {
            asm_block_t* asm_block = new asm_block_t;

            asm_block->line = m_current.line;
            asm_block->offset = m_current.offset;
            asm_block->len = m_current.text.size();
            asm_block->assembly = m_current.text;

            m_current = m_input->get();

            return asm_block;
        }

        expression_t* parse_expression_block() {
            expression_block_t* block = new expression_block_t;

            block->line = m_current.line;
            block->offset = m_current.offset;
            block->len = m_current.text.size();

            expression_t* lhs;
            expression_t* op;

            m_current = m_input->get();

            while (m_current.type != LT_CLOSING_BRACE) {
                lhs = parse_expression();

                do {
                    op = parse_rightside_operation(lhs);

                    if (op) lhs = op;
                } while (op);

                block->block.push_back(lhs);

                if (m_current.type != LT_SEMICOLON) {
                    ERROR("Expressions on blocks must be separated by semicolons");
                }

                m_current = m_input->get();
            }

            m_current = m_input->get();

            return block;
        }

        expression_t* parse_rightside_operation(expression_t* lhs) {
            switch (m_current.type) {
                case LT_OPERATOR_BINARY: case LT_STAR: case LT_AMPERSAND: {
                    return parse_binary_op(lhs);
                } break;

                case LT_OPERATOR_COMP: {
                    return parse_comp_op(lhs);
                } break;

                case LT_OPENING_PARENT: {
                    m_current = m_input->get();

                    return parse_function_call(lhs);
                } break;

                case LT_OPENING_BRACKET: {
                    m_current = m_input->get();

                    return parse_array_access(lhs);
                } break;

                case LT_OPERATOR_ASSIGN: {
                    return parse_assignment(lhs);
                } break;

                default: return nullptr;
            }

            return nullptr;
        }
=======
        std::unordered_map <parser_expression_t, expression_parser_t> m_expression_parsers = {
            { PT_FUNCTION_DEF, parse_function_def }
        };
>>>>>>> Stashed changes

        parser_output_t* get_output() {
            return &m_output;
        }

<<<<<<< Updated upstream
=======
        inline bool is_token(lexer_token_type_t lt) {
            return m_current.type == lt;
        }

        bool expect_token(lexer_token_type_t lt) {
            if (!is_token(lt)) {
                ERROR(fmt("Expected " ESCAPE(37) "%s" ESCAPE(0) ", got " ESCAPE(37) "\'%s\'" ESCAPE(0) " instead",
                    lexer_token_hr_names[lt],
                    m_current.text.c_str()
                ));

                std::exit(1);
            }

            return true;
        }

        expression_t* expect_expr(parser_expression_t pt, expression_t* arg) {
            expression_parser_t p = m_expression_parsers[pt];

            assert(p);

            expression_t* expr = p(arg);

            if (!expr) {
                ERROR(fmt("Expected " ESCAPE(37) "\'%s\'", parser_expression_hr_names[pt]));

                return false;
            }

            return expr;
        }

        expression_t* try_expr(parser_expression_t pt, expression_t* arg) {
            expression_parser_t p = m_expression_parsers[pt];

            assert(p);

            size_t pos = m_input->tellg();

            expression_t* expr = p(arg);

            if (!expr) {
                m_input->seekg(pos);

                return nullptr;
            }

            return expr;
        }

        inline void consume() {
            m_current = m_input->get();
        }
    
        // fn: expr
        // fn name: expr
        // fn (arg-list): expr
        // fn -> type: expr
        // fn name(arg-list): expr
        // fn name -> type: expr
        // fn (arg-list) -> type: expr
        expression_t* parse_function_def(expression_t* prev = nullptr) {
            if (!is_token(LT_KEYWORD_FN)) {
                return nullptr;
            }

            consume();

            _log(debug, "Found \'%s\'", lexer_token_hr_names[m_current.type]);

            return nullptr;

            switch (m_current.type) {
                // Anonymous without arg-list nor return-type
                case LT_COLON: {
                    
                } break;

                // Named
                case LT_IDENT: {

                } break;

                // Anonymous with arg-list 
                case LT_OPENING_PARENT: {

                } break;

                // Anonymous with return-type but no arg-list
                case LT_ARROW: {

                } break;
            }
        }

        expression_t* parse_expression_impl() {
            expression_t* expr;
        
            switch (m_current.type) {
                case LT_KEYWORD_FN: {
                    expr = parse_function_def();
                } break;

                // Could be many things...
                case LT_IDENT: {
                    if (!ts.exists(m_current.text)) {
                        // Name ref
                    } else {
                        // Definition
                        hs_type_t* type = parse_type();
                    }
                } break;

                case LT_KEYWORD_MUT: {
                    hs_type_t* type = parse_type();
                } break;

                default: {
                    ERROR(fmt("Unhandled token \"" ESCAPE(37;1) "%s" ESCAPE(0) "\"", m_current.text.c_str()));

                    expr = nullptr;
                };
            }

            return expr;
        }

        inline bool is_type_modifier() {
            return (m_current.type == LT_KEYWORD_MUT    ) ||
                   (m_current.type == LT_KEYWORD_STATIC ) ||
                   (m_current.type == LT_KEYWORD_CONST  );
        }

        hs_type_t* parse_type() {
            std::string signature;

            bool mut, is_static;

            hs_type_t* type;

            while (is_type_modifier()) {
                switch (m_current.type) {
                    case LT_KEYWORD_MUT   : mut = true; break;
                    case LT_KEYWORD_STATIC: is_static = true; break;
                    case LT_KEYWORD_CONST : mut = false; break;
                }

                signature.append(m_current.text);
                signature.push_back(' ');

                m_current = m_input->get();
            }

            if (!ts.exists(m_current.text)) {
                // Error: expected a type after type modifier

                return nullptr;
            }

            type = ts.get_type(m_current.text);

            signature.append(m_current.text);

            m_current = m_input->get();

            while (m_current.type == LT_STAR) {
                type = new pointer_type_t(type);

                signature.push_back('*');

                m_current = m_input->get();
            }

            type->mut = mut;
            type->is_static = is_static;

            _log(debug, "signature=%s", signature.c_str());

            ts.add_type(signature, type);
        }

        // -- Support functions --

        expression_t* parse_rhs(expression_t* lhs) {
            expression_t* expr = lhs;

            if (m_current.type == LT_OPERATOR_UNARY) {
                unary_op_t* uo = new unary_op_t;

                uo->op = m_current.text;
                uo->post = true;
                uo->operand = lhs;

                m_current = m_input->get();

                expr = uo;
            }

            if ((m_current.type == LT_OPERATOR_BINARY) || (m_current.type == LT_STAR) || (m_current.type == LT_AMPERSAND)) {
                binary_op_t* bo = new binary_op_t;

                bo->op = m_current.text;
                bo->lhs = lhs;

                m_current = m_input->get();

                bo->rhs = parse_expression();

                expr = bo;
            }

            if (m_current.type == LT_OPERATOR_COMP) {
                comp_op_t* co = new comp_op_t;

                co->op = m_current.text;
                co->lhs = lhs;

                m_current = m_input->get();

                co->rhs = parse_expression();

                expr = co;
            }

            if (m_current.type == LT_OPERATOR_ASSIGN) {
                assignment_t* as = new assignment_t;

                as->assignee = lhs;
                as->op = m_current.text;

                m_current = m_input->get();

                as->value = parse_expression();

                expr = as;
            }

            if (m_current.type == LT_OPENING_PARENT) {
                function_call_t* fc = new function_call_t;

                m_current = m_input->get();

                // To-do
                //expr = parse_function_call(lhs);
            }

            if (m_current.type == LT_OPENING_BRACKET) {
                array_access_t* aa = new array_access_t;

                m_current = m_input->get();

                // To-do
                //expr = parse_array_access(lhs);
            }

            return expr;
        }

        expression_t* parse_expression() {
            expression_t* expr = nullptr;

            bool parenthesized = m_current.type == LT_OPENING_PARENT;

            if (parenthesized) {
                m_current = m_input->get();

                expr = parse_expression();

                if (m_current.type != LT_CLOSING_PARENT) {
                    ERROR("Expected ')'");
                }

                m_current = m_input->get();

                expr = parse_rhs(expr);
            } else {
                // Parse pre-unary op
                if (m_current.type == LT_OPERATOR_UNARY) {
                    unary_op_t* uo = new unary_op_t;

                    uo->op = m_current.text;
                    uo->post = false;

                    m_current = m_input->get();

                    uo->operand = parse_expression();

                    expr = uo;
                } else {
                    expr = parse_expression_impl();
                }

                expr = parse_rhs(expr);
            }

            return expr;
        }

>>>>>>> Stashed changes
        bool parse() {
            expression_t* lhs;
            expression_t* op;

            m_current = m_input->get();

            while (m_current.type != LT_NONE) {
                lhs = parse_expression();

                do {
                    op = parse_rightside_operation(lhs);

                    if (op) lhs = op;
                } while (op);

                // Handle ; thing
                if (lhs == (expression_t*)10) {
                    m_current = m_input->get();

                    continue;
                }

                if (lhs) {
                    m_output.source.push_back(lhs);

                    //_log(debug, "expression:\n%s", lhs->print(0).c_str());
                } else {
                    return false;
                }

                m_current = m_input->get();
            }

            return true;
        }
    };
}

hs::expression_t* hs::parser_t::parse_expression_impl() {
    hs::expression_t* expr;
 
    switch (m_current.type) {
        case LT_KEYWORD_FN: {
            expr = parse_function_def();
        } break;

        case LT_KEYWORD_WHILE: {
            expr = parse_while_loop();
        } break;

        case LT_KEYWORD_RETURN: {
            expr = parse_return();
        } break;

        case LT_KEYWORD_ARRAY: {
            expr = parse_array();
        } break;

        case LT_KEYWORD_BLOB: {
            expr = parse_blob();
        } break;

        case LT_KEYWORD_IF: {
            expr = parse_if_else();
        } break;

        case LT_LITERAL_NUMERIC: {
            expr = parse_numeric_literal();
        } break;

        case LT_LITERAL_STRING: {
            expr = parse_string_literal();
        } break;

        case LT_KEYWORD_INVOKE: {
            expr = parse_invoke();
        } break;

        case LT_OPENING_BRACKET: {
            type_t* none = new type_t;

            none->line = m_current.line;
            none->offset = m_current.offset;
            none->len = m_current.text.size();

            none->type = "none";

            m_current = m_input->get();

            expr = parse_array_access(none);
        } break;

        case LT_IDENT: {
            bool type = is_type(m_current.text);

            if (type) {
                std::string type = m_current.text;

                m_current = m_input->get();

                if (m_current.type == LT_IDENT) {
                    variable_def_t* var = new variable_def_t;

                    var->line = m_current.line;
                    var->offset = m_current.offset;
                    var->len = m_current.text.size();

                    var->type = type;
                    var->name = m_current.text;

                    expr = var;

                    m_current = m_input->get();
                } else {
                    type_t* type_expr = new type_t;

                    type_expr->line = m_current.line;
                    type_expr->offset = m_current.offset;
                    type_expr->len = m_current.text.size();

                    type_expr->type = type;

                    expr = type_expr;
                }
            } else {
                name_ref_t* name = new name_ref_t;

                name->line = m_current.line;
                name->offset = m_current.offset;
                name->len = m_current.text.size();

                name->name = m_current.text;

                expr = name;

                m_current = m_input->get();
            }
        } break;

        case LT_OPENING_BRACE: {
            expr = parse_expression_block();
        } break;

        case LT_ASM_BLOCK: {
            expr = parse_asm_block();
        } break;

        case LT_SEMICOLON: {
            return (hs::expression_t*)10;
        } break;

        default: {
            ERROR(fmt("Unhandled token \"" ESCAPE(37;1) "%s" ESCAPE(0) "\"", m_current.text.c_str()));

            expr = nullptr;
        };
    }

    return expr;
}

hs::expression_t* hs::parser_t::parse_expression() {
    expression_t* lhs;
    expression_t* op;

    bool parenthesized = m_current.type == LT_OPENING_PARENT;

    if (parenthesized) {
        parenthesized = true;

        m_current = m_input->get();

        lhs = hs::parser_t::parse_expression();
    } else {
        lhs = hs::parser_t::parse_expression_impl();
    }

    do {
        op = parse_rightside_operation(lhs);

        if (op) lhs = op;
    } while (op);
    
    if (parenthesized) {
        if (m_current.type != LT_CLOSING_PARENT) {
            ERROR("Expected \'" ESCAPE(37;1) ")" ESCAPE(0)"\'");

            return nullptr;
        } else {
            m_current = m_input->get();
        }
    }

    return lhs;
}

#undef ERROR