#pragma once
#include "common.h"
#include "Parser/Parser.h"

class InstructionEncoder {
private:
    const std::unordered_map<std::string, uint16_t> OPCODE_MAP = {
        {"mv", 0}, {"b", 1}, {"beq", 1}, {"bne", 1}, {"bcc", 1}, 
        {"bcs", 1}, {"bpl", 1}, {"bmi", 1}, {"bl", 1}, {"mvt", 1},
        {"add", 2}, {"sub", 3}, {"ld", 4}, {"pop", 4}, 
        {"st", 5}, {"push", 5}, {"and", 6}, {"cmp", 7},
        {"lsl", 7}, {"lsr", 7}, {"asr", 7}, {"ror", 7}
    };

    const std::unordered_map<std::string, uint16_t> REG_MAP = {
        {"r0", 0}, {"r1", 1}, {"r2", 2}, {"r3", 3},
        {"r4", 4}, {"r5", 5}, {"sp", 5}, {"r6", 6},
        {"lr", 6}, {"r7", 7}, {"pc", 7}
    };

    const std::unordered_map<std::string, uint16_t> CONDITION_MAP = {
        {"", 0}, {"eq", 1}, {"ne", 2}, {"cc", 3},
        {"cs", 4}, {"pl", 5}, {"mi", 6}, {"l", 7}
    };

    const std::unordered_map<std::string, uint16_t> SHIFT_TYPE_MAP = {
        {"lsl", 0}, {"lsr", 1}, {"asr", 2}, {"ror", 3}
    };

    uint16_t encodeRegisterOperation(uint16_t opcode, uint16_t rX, uint16_t rY) {
        return rY | (rX << 9) | (opcode << 13);
    }

    uint16_t encodeImmediateOperation(uint16_t opcode, uint16_t rX, uint16_t imm) {
        return (imm & 0x1FF) | (rX << 9) | (1 << 12) | (opcode << 13);
    }

    uint16_t encodeMvtOperation(uint16_t rX, uint16_t imm) {
        return (imm) | (rX << 9) | (1 << 12) | (1 << 13);
    }

    uint16_t encodeBranchOperation(uint16_t condition, int16_t offset) {
        uint16_t value;
        if (offset > 0) {
            value = offset & 0x1FF;
        } else {
            value = ((0x1FF ^ (-offset)) + 1) & 0x1FF;
        }
        return value | (condition << 9) | (1 << 13);
    }

    uint16_t encodeShiftRegister(uint16_t opcode, uint16_t rX, uint16_t rY, uint16_t shiftType) {
        return rY | (shiftType << 5) | (1 << 8) | (rX << 9) | (opcode << 13);
    }

    uint16_t encodeShiftImmediate(uint16_t opcode, uint16_t rX, uint16_t imm, uint16_t shiftType) {
        return (imm & 0xF) | (shiftType << 5) | (1 << 7) | (1 << 8) | 
               (rX << 9) | (opcode << 13);
    }

public:
    std::vector<uint16_t> encode(const std::string& operation, const std::string& op1, const std::string& op2, int currentAddress = 0, int targetAddress = 0);
};