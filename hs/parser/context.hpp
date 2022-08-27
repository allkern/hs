#pragma once

#include "parser.hpp"
#include "output.hpp"

namespace hs {
    struct contextualizer_t {
        parser_output_t* m_po;

        void init(parser_t* parser) {
            m_po = parser->get_output();
        }
    };
}