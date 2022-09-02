#pragma once

#include "../instruction.hpp"
#include "../generator.hpp"

#include "../../error.hpp"

#include <vector>

namespace hs {
    class ir_translator_t {
    public:
        virtual void init(hs::ir_generator_t*, hs::error_logger_t*) {};
        virtual std::string translate() { return ""; };
    };
}