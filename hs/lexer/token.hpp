#pragma once

#include <string>
#include <unordered_map>

namespace hs {
    enum lexer_token_type_t : int {
        LT_NONE,
        LT_IDENT,
        LT_LITERAL,
        LT_LITERAL_NUMERIC,
        LT_LITERAL_STRING,
        LT_OPENING_PARENT,
        LT_CLOSING_PARENT,
        LT_OPENING_BRACE,
        LT_CLOSING_BRACE,
        LT_OPENING_BRACKET,
        LT_CLOSING_BRACKET,
        LT_COLON,
        LT_SEMICOLON,
        LT_ARROW,
        LT_COMMA,
        LT_DOT,
        LT_STAR,
        LT_AMPERSAND,
        LT_OPERATOR_ASSIGN,
        LT_OPERATOR_COMP,
        LT_OPERATOR_BINARY,
        LT_OPERATOR_UNARY,
        LT_KEYWORD_FN,
        LT_KEYWORD_RETURN,
        LT_KEYWORD_CONST,
        LT_KEYWORD_GENERIC,
        LT_KEYWORD_FOR,
        LT_KEYWORD_IF,
        LT_KEYWORD_ELSE,
        LT_KEYWORD_WHILE,
        LT_KEYWORD_DO,
        LT_KEYWORD_INVOKE,
        LT_KEYWORD_RANGE,
        LT_KEYWORD_STRUCT,
        LT_KEYWORD_TYPE,
        LT_ASM_BLOCK
    };

    std::unordered_map <std::string, lexer_token_type_t> keyword_map = {
        { "fn"      , LT_KEYWORD_FN      },
        { "return"  , LT_KEYWORD_RETURN  },
        { "const"   , LT_KEYWORD_CONST   },
        { "generic" , LT_KEYWORD_GENERIC },
        { "for"     , LT_KEYWORD_FOR     },
        { "if"      , LT_KEYWORD_IF      },
        { "else"    , LT_KEYWORD_ELSE    },
        { "while"   , LT_KEYWORD_WHILE   },
        { "do"      , LT_KEYWORD_DO      },
        { "invoke"  , LT_KEYWORD_INVOKE  },
        { "range"   , LT_KEYWORD_RANGE   },
        { "struct"  , LT_KEYWORD_STRUCT  },
        { "type"    , LT_KEYWORD_TYPE    }
    };

    std::string lexer_token_type_names[] = {
        "LT_NONE",
        "LT_IDENT",
        "LT_LITERAL",
        "LT_LITERAL_NUMERIC",
        "LT_LITERAL_STRING",
        "LT_OPENING_PARENT",
        "LT_CLOSING_PARENT",
        "LT_OPENING_BRACE",
        "LT_CLOSING_BRACE",
        "LT_OPENING_BRACKET",
        "LT_CLOSING_BRACKET",
        "LT_COLON",
        "LT_SEMICOLON",
        "LT_ARROW",
        "LT_COMMA",
        "LT_DOT",
        "LT_STAR",
        "LT_AMPERSAND",
        "LT_OPERATOR_ASSIGN",
        "LT_OPERATOR_COMP",
        "LT_OPERATOR_BINARY",
        "LT_OPERATOR_UNARY",
        "LT_KEYWORD_FN",
        "LT_KEYWORD_RETURN",
        "LT_KEYWORD_CONST",
        "LT_KEYWORD_GENERIC",
        "LT_KEYWORD_FOR",
        "LT_KEYWORD_IF",
        "LT_KEYWORD_ELSE",
        "LT_KEYWORD_WHILE",
        "LT_KEYWORD_DO",
        "LT_KEYWORD_INVOKE",
        "LT_KEYWORD_RANGE",
        "LT_KEYWORD_STRUCT",
        "LT_KEYWORD_TYPE",
        "LT_ASM_BLOCK"
    };

    struct lexer_token_t {
        unsigned int line = 0, offset = 0;

        lexer_token_type_t type = LT_NONE;

        std::string text;
    };
}