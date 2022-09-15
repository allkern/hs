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
        IR_PUSHR,       // Push register
        IR_POPR,        // Pop register
        IR_LEAF,        // Push register
        IR_RET,         // Return from subroutine
        IR_ALU,         // ALU instruction
        IR_FPU,         // FPU instruction
        IR_BRANCH,      // Branch (ne, eq, etc.)
        IR_DEFINE,      // Assembly define
        IR_PASSTHROUGH, // Passthrough, will emit its first argument verbatim
        IR_NOP          // NOP
    };

    struct ir_instruction_t {
        ir_opcode_t opcode;

        std::string args[4];
    };
}