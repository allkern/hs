#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <stack>

#include "hs/lexer/lexer.hpp"
#include "hs/parser/parser.hpp"

// -> a -> b -> c  abc

// fn name opening_parent ... closing_parent arrow type colon expression
// type name eq_operator expression semicolon
// name opening_parent ... closing_parent semicolon
// opening_curly ... closing_curly
// type opening_bracket ... closing_bracket

// fn main(u32 a) -> u32: a;
// u32 a = 100;
// main();
// { a; }
// u32 [100];

int main(int argc, const char* argv[]) {
    _log::init("hs");

    std::string filename;

    if (argv[1]) {
        filename = std::string(argv[1]);
    } else {
        filename = "test.hs";
    }

    std::ifstream file(filename, std::ios::binary);

    hs::lexer_t lexer;
    hs::parser_t parser;
    hs::error_logger_t error_logger;

    error_logger.init(&file, filename);
    lexer.init(&file, &error_logger);
    lexer.lex();

    parser.init(lexer.get_output(), &error_logger);

    // while (!lexer.eof()) {
    //     hs::lexer_token_t token = lexer.get();

    //     // error_logger.print_error("main", "Successfully lexed token!", token.line, token.offset, token.text.size());

    //     std::cout << "(" << token.line + 1 << ", " << token.offset + 1 << ": type: " << hs::lexer_token_type_names[token.type] << ", text: " << token.text << ")\n";
    // }

    parser.parse();

    return 0;
}