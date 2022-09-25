#pragma once

#include <string>

namespace hs {
    enum ir_opcode_t {
        IR_LABEL,       // Generate a label
        IR_MOV,         // Move registers
        IR_MOVI,        // Move register to immediate
        IR_LOADR,       // Load register from register
        IR_LOADF,       // Load register from frame
        IR_STORE,       // Store register on register
        IR_ADDSP,       // Add Immediate to Stack Pointer
        IR_SUBSP,       // Subtract Immediate from Stack Pointer
        IR_ADDFP,       // Add Immediate to Frame Pointer
        IR_DECSP,       // Decrement Stack Pointer
        IR_CALLR,       // Call register
        IR_CMPRI,       // Compare register with immediate
        IR_PUSHR,       // Push register
        IR_POPR,        // Pop register
        IR_LEAF,        // Push register
        IR_RET,         // Return from subroutine
        IR_ALU,         // ALU instruction
        IR_FPU,         // FPU instruction
        IR_BRANCH,      // Branch (ne, eq, etc.)
        IR_DEFINE,      // Assembly define
        IR_UNDEF,       // Assembly undef
        IR_DEFSTR,      // Define string
        IR_DEFV,        // Define value
        IR_PASSTHROUGH, // Passthrough, will emit its first argument verbatim
        IR_NOP          // NOP
    };

    std::string m_ir_mnemonic_map[] = {
        "LABEL",
        "MOV",
        "MOVI",
        "LOADR",
        "LOADF",
        "STORE",
        "ADDSP",
        "SUBSP",
        "ADDFP",
        "DECSP",
        "CALLR",
        "CMPRI",
        "PUSHR",
        "POPR",
        "LEAF",
        "RET",
        "ALU",
        "FPU",
        "BRANCH",
        "DEFINE",
        "UNDEF",
        "DEFSTR",
        "DEFV",
        "PASSTHROUGH",
        "NOP"
    };

    struct ir_instruction_t {
        ir_opcode_t opcode;

        std::string args[4];
    };

    std::string print_ir_instruction(ir_instruction_t& ir) {
        std::ostringstream ss;

        ss << m_ir_mnemonic_map[ir.opcode] << " ";

        int i = 0;

        while (ir.args[i].size()) {
            ss << ir.args[i++] << ", ";
        }

        return ss.str();
    }
}