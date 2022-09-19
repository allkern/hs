#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <stack>

#include "hs/compiler.hpp"

int main(int argc, const char* argv[]) {
    _log::init("hs");

    hs::compiler_t compiler;

    if (!compiler.init(argc, argv)) {
        return -1;
    }

    if (!compiler.compile()) {
        return -1;
    }

    return 0;
}
