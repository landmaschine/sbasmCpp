#include  "InstructionEncoder.h"


std::vector<uint16_t> InstructionEncoder::encode(const std::string& operation, const std::string& op1, const std::string& op2, int currentAddress, int targetAddress) {
    std::vector<uint16_t> result;

    auto opcode = OPCODE_MAP.at(operation);

    if(operation == "mv" && op2[0] == '=') {
        auto rX = REG_MAP.at(op1);
        auto imm = std::stoi(op2.substr(1), nullptr, 0);

        uint16_t high = (imm >> 8) & 0xFF;
        uint16_t low = imm & 0xFF;

        result.push_back(encodeMvtOperation(rX, high));
        result.push_back(encodeImmediateOperation(OPCODE_MAP.at("add"), rX, low));
    } else if(operation[0] == 'b') {
        std::string condition = operation.substr(1);
        auto condValue = CONDITION_MAP.at(condition);
        int offset = targetAddress - (currentAddress + 1);

        result.push_back(encodeBranchOperation(condValue, offset));
    } else if(operation == "lsl" || operation == "lsr" || operation == "asr" || operation == "ror") {
        auto rX = REG_MAP.at(op1);
        auto shiftType = SHIFT_TYPE_MAP.at(operation);

        if(op2[0] == '#' || op2[0] == '=') {
            auto imm = std::stoi(op2.substr(1), nullptr, 0);
            result.push_back(encodeShiftImmediate(opcode, rX, imm, shiftType));
        } else {
            auto rY = REG_MAP.at(op2);
            result.push_back(encodeShiftRegister(opcode, rX, rY, shiftType));
        }
    } else if (op2[0] == '#' || op2[0] == '=') {
        auto rX = REG_MAP.at(op1);
        auto imm = std::stoi(op2.substr(1), nullptr, 0);

        if(operation == "mvt") {
            result.push_back(encodeMvtOperation(rX, imm));
        } else {
            result.push_back(encodeImmediateOperation(opcode, rX, imm));
        }
    } else {
        auto rX = REG_MAP.at(op1);
        auto rY = REG_MAP.at(op2);
        result.push_back(encodeRegisterOperation(opcode, rX, rY));
    }

    return result;
}