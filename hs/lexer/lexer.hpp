#pragma once

#include <iostream>
#include <string>
#include <cctype>
#include <unordered_map>

#include "../stream.hpp"
#include "../error.hpp"

#include "token.hpp"

namespace hs {
    class lexer_t {
        std::istream* m_input;
        stream_t <lexer_token_t> m_output;

        error_logger_t* m_logger;

        unsigned int m_line = 0, m_offset = 0;

    public:
        enum lex_status_t : int {
            ST_NO_MATCH,
            ST_MATCH,
            ST_ERROR
        };

        struct result_t {
            lex_status_t status;

            std::string error;

            unsigned int line = 0, offset = 0, len = 1;
        };

        typedef result_t (*lex_fn_t)(void);

        char m_current;

        lexer_token_t m_current_token;

        void init(std::istream* input, error_logger_t* logger) {
            m_input = input;

            m_current = m_input->get();
            m_logger = logger;
        }

#define CONSUME { m_current = m_input->get(); m_offset++; } 
#define MATCH { ST_MATCH, "" };
#define NO_MATCH { ST_NO_MATCH, "" };
#define ERROR(msg, errl) { ST_ERROR, msg, m_line, m_offset, errl };

        result_t lex_asm_block() {
            m_current_token.type = LT_ASM_BLOCK;

            ignore_whitespace();

            if (m_current != '{') {
                return ERROR("Expected '{' after asm block declaration", 1);
            }

            int matching_braces = 1;

            m_current_token.text.clear();

            CONSUME;

            while (matching_braces) {
                if (m_current == '\n') {
                    m_offset = 0;
                    m_line++;
                }

                if (m_current == '{') matching_braces++;
                if (m_current == '}') matching_braces--;

                if (m_current == '}' && !matching_braces) {
                    break;
                }

                m_current_token.text.push_back(m_current);

                CONSUME;
            }

            CONSUME;

            return MATCH;
        }

        result_t try_lex_identifier() {
            if (!(std::isalpha(m_current) || m_current == '_')) return NO_MATCH;

            m_current_token.line = m_line;
            m_current_token.offset = m_offset;

            m_current_token.text.push_back(m_current);

            CONSUME;

            while (std::isalnum(m_current) || m_current == '_') {
                m_current_token.text.push_back(m_current);

                CONSUME;
            }

            if (m_current_token.text == "asm") {
                return lex_asm_block();
            }

            m_current_token.type = LT_IDENT;

            return MATCH;
        }

#define SINGLE(c) \
    m_current_token.text = c; \
    m_current_token.line = m_line; \
    m_current_token.offset = m_offset; \
    CONSUME;


        result_t try_lex_structural() {
            switch (m_current) {
                case '{': SINGLE("{"); m_current_token.type = LT_OPENING_BRACE  ; return MATCH;
                case '}': SINGLE("}"); m_current_token.type = LT_CLOSING_BRACE  ; return MATCH;
                case '(': SINGLE("("); m_current_token.type = LT_OPENING_PARENT ; return MATCH;
                case ')': SINGLE(")"); m_current_token.type = LT_CLOSING_PARENT ; return MATCH;
                case '[': SINGLE("["); m_current_token.type = LT_OPENING_BRACKET; return MATCH;
                case ']': SINGLE("]"); m_current_token.type = LT_CLOSING_BRACKET; return MATCH;
                case ':': SINGLE(":"); m_current_token.type = LT_COLON          ; return MATCH;
                case ';': SINGLE(";"); m_current_token.type = LT_SEMICOLON      ; return MATCH;
                case '.': SINGLE("."); m_current_token.type = LT_DOT            ; return MATCH;
                case ',': SINGLE(","); m_current_token.type = LT_COMMA          ; return MATCH;
            }

            return NO_MATCH;
        }

        result_t try_lex_operator() {
            char next = m_input->peek();

            switch (m_current) {
                case '+': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "+=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        case '+': {
                            m_current_token.text = "++";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_UNARY;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('+');

                            m_current_token.type = LT_OPERATOR_BINARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '-': {
                    switch (next) {
                        case '>': {
                            m_current_token.text = "->";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_ARROW;

                            CONSUME; CONSUME;
                        } break;

                        case '-': {
                            m_current_token.text = "--";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_UNARY;

                            CONSUME; CONSUME;
                        } break;

                        case '=': {
                            m_current_token.text = "-=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('-');

                            m_current_token.type = LT_OPERATOR_BINARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '*': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "*=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('*');

                            m_current_token.type = LT_STAR;
                        } break;
                    }

                    return MATCH;
                } break;

                case '/': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "/=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('/');

                            m_current_token.type = LT_OPERATOR_BINARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '&': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "&=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        case '&': {
                            m_current_token.text = "&&";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_COMP;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('&');

                            m_current_token.type = LT_AMPERSAND;
                        } break;
                    }

                    return MATCH;
                } break;

                case '|': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "|=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        case '|': {
                            m_current_token.text = "||";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_COMP;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('|');

                            m_current_token.type = LT_OPERATOR_BINARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '^': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "^=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;

                        case '^': {
                            m_current_token.text = "^^";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_COMP;

                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('^');

                            m_current_token.type = LT_OPERATOR_BINARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '!': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "!=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_COMP;
                            
                            CONSUME; CONSUME;
                        } break;

                        default: {
                            SINGLE('!');

                            m_current_token.type = LT_OPERATOR_UNARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '%': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "%=";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_ASSIGN;

                            CONSUME; CONSUME;
                        } break;
                        default: {
                            SINGLE('%');

                            m_current_token.type = LT_OPERATOR_BINARY;
                        } break;
                    }

                    return MATCH;
                } break;

                case '=': {
                    switch (next) {
                        case '=': {
                            m_current_token.text = "==";
                            m_current_token.line = m_line;
                            m_current_token.offset = m_offset;
                            m_current_token.type = LT_OPERATOR_COMP;

                            CONSUME; CONSUME;
                        } break;
                        default: {
                            SINGLE("=");

                            m_current_token.type = LT_OPERATOR_ASSIGN;
                        } break;
                    }

                    return MATCH;
                } break;

                case '~': SINGLE('~'); m_current_token.type = LT_OPERATOR_UNARY ; return MATCH;
            }

            return NO_MATCH;
        }

#undef SINGLE

        result_t try_lex_literal() {
            if (std::isdigit(m_current)) {
                // Parse numeric literal

                m_current_token.line = m_line;
                m_current_token.offset = m_offset;

                m_current_token.text.push_back(m_current);

                CONSUME;

                while (std::isxdigit(m_current) || m_current == 'b' || m_current == 'x' || m_current == '.') {
                    m_current_token.text.push_back(m_current);

                    CONSUME;
                }

                m_current_token.type = LT_LITERAL_NUMERIC;

                return MATCH;
            } else if (m_current == '\"') {
                // Parse string literal

                m_current_token.line = m_line;
                m_current_token.offset = m_offset;

                CONSUME;

                while (m_current != '\"') {
                    m_current_token.text.push_back(m_current);

                    CONSUME;
                }

                CONSUME;

                m_current_token.type = LT_LITERAL_STRING;

                return MATCH;
            } else if (m_current == '\'') {
                // Parse char literal

                m_current_token.line = m_line;
                m_current_token.offset = m_offset;

                CONSUME;

                m_current_token.text = std::to_string((int)m_current);

                CONSUME;

                if (m_current != '\'')
                    return ERROR("Expected " ESCAPE(37;1) "\'" ESCAPE(0) " after char constant", 1);

                CONSUME;

                m_current_token.type = LT_LITERAL_NUMERIC;

                return MATCH;
            } else {
                return NO_MATCH;
            }
        }

        void ignore_whitespace() {
            while (std::isspace(m_current)) {
                if (m_current == '\n') {
                    m_offset = 0;
                    m_line++;
                } else {
                    m_offset++;
                }

                m_current = m_input->get();
            }
        }

        lexer_token_t get() {
            return m_output.get();
        }

        bool eof() {
            return m_output.eof();
        }

        stream_t <lexer_token_t>* get_output() {
            return &m_output;
        }

#undef CONSUME
#undef MATCH
#undef NO_MATCH
#undef ERROR

#define TRY_LEX(lt) \
    r = try_lex_##lt(); \
\
    if (r.status == ST_MATCH) { \
        m_output.put(m_current_token); \
\
        continue; \
    } \
    if (r.status == ST_ERROR) { \
        if (m_logger) m_logger->print_error( \
            "lexer", \
            r.error, \
            r.line, r.offset, 1 \
        ); \
\
        return; \
    }

        bool fix_keyword(lexer_token_t* token) {
            if (token->type != LT_IDENT) return false;

            if (keyword_map.contains(token->text)) {
                token->type = keyword_map[token->text];

                return true;
            }

            return false;
        }

        void fix_types() {
            for (auto& token : m_output) {
                if (fix_keyword(&token)) continue;
            }
        }

        void lex() {
            result_t r;

            while (!m_input->eof()) {
                ignore_whitespace();

                m_current_token.text.clear();

                TRY_LEX(identifier);
                TRY_LEX(literal);
                TRY_LEX(structural);
                TRY_LEX(operator);

                if (m_logger) m_logger->print_error(
                    "lexer",
                    fmt("Unexpected character \'" ESCAPE(37;1) "%c" ESCAPE(0) "\'", m_current),
                    m_line, m_offset, 1
                );

                return;
            }

            fix_types();
        }
    };
}