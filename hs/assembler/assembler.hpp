#pragma once

#include "../error.hpp"
#include "../cli.hpp"

#include <vector>

namespace hs {
    class assembler_t {
    public:
        virtual void init(std::istream*, std::ostream*, error_logger_t*, cli_parser_t*) {};
        virtual void assemble() {};
    };
}