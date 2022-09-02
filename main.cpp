#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <stack>

#include "hs/lexer/lexer.hpp"
#include "hs/parser/parser.hpp"
#include "hs/parser/context.hpp"
#include "hs/preprocessor/preprocessor.hpp"
#include "hs/ir/generator.hpp"
#include "hs/ir/instruction.hpp"
#include "hs/ir/translators/hyrisc.hpp"

int main(int argc, const char* argv[]) {
    _log::init("hs");

    std::string filename;

    if (argv[1]) {
        filename = std::string(argv[1]);
    } else {
        filename = "test.hs";
    }

    std::ifstream file(filename, std::ios::binary);

    hs::preprocessor_t preprocessor;
    hs::error_logger_t error_logger;
    hs::lexer_t lexer;
    hs::parser_t parser;
    hs::contextualizer_t contextualizer;
    hs::ir_generator_t ir_generator;

    preprocessor.init(&file, &error_logger);
    preprocessor.preprocess();

    std::stringstream* output = preprocessor.get_output();

    error_logger.init(output, filename);

    lexer.init(output, &error_logger);
    lexer.lex();

    parser.init(lexer.get_output(), &error_logger);

    // while (!lexer.eof()) {
    //     hs::lexer_token_t token = lexer.get();

    //     // error_logger.print_error("main", "Successfully lexed token!", token.line, token.offset, token.text.size());

    //     std::cout << "(" << token.line + 1 << ", " << token.offset + 1 << ": type: " << hs::lexer_token_type_names[token.type] << ", text: " << token.text << ")\n";
    // }

    parser.parse();

    contextualizer.init(&parser, &error_logger);
    contextualizer.contextualize();

    ir_generator.init(&parser, &error_logger);

    ir_generator.generate();

    hs::ir_tr_hyrisc_t translator;

    translator.init(&ir_generator, &error_logger);

    std::cout << translator.translate();

    return 0;
}