#pragma once

#include <vector>
#include <fstream>
#include <cstdio>

#include "../../error.hpp"
#include "../assembler.hpp"

namespace hs {
    class assembler_x86_64_t : public assembler_t {
        std::istream* m_input;
        std::ostream* m_output;
        error_logger_t* m_logger;
        std::ofstream m_temp_out_file;
        std::ifstream m_temp_in_file;

    public:
        void init(std::istream* input, std::ostream* output, error_logger_t* logger, cli_parser_t*) override {
            m_input = input;
            m_output = output;
            m_logger = logger;

            m_temp_out_file.open("__TEMP_ASSEMBLY__", std::ios::binary);

            while (!m_input->eof())
                m_temp_out_file.put(m_input->get());
        }

        void assemble() {
            system("as __TEMP_ASSEMBLY__ -o __TEMP_ASSEMBLED__");

            m_temp_out_file.close();

            std::remove("__TEMP_ASSEMBLY__");

            m_temp_in_file.open("__TEMP_ASSEMBLED__", std::ios::binary);

            while (!m_temp_in_file.eof())
                m_output->put(m_temp_in_file.get());

            m_temp_in_file.close();

            std::remove("__TEMP_ASSEMBLED__");
        };
    };
}