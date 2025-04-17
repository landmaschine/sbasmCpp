# Guide to Adding Instructions to the qCore Assembler

This document provides a comprehensive step-by-step guide on how to extend the qCore assembler with new instructions, using the XOR instruction as a practical example.

## Table of Contents
1. [Assembler Architecture](#assembler-architecture)
2. [Instruction Encoding Format](#instruction-encoding-format)
3. [Step-by-Step Process](#step-by-step-process)
4. [XOR Implementation Example](#xor-implementation-example)
5. [Testing and Debugging](#testing-and-debugging)

## Assembler Architecture

The qCore assembler follows a typical compiler pipeline with these main components:

- **Lexer** (`Lexer.h/cpp`): Converts raw text into tokens
- **Parser** (`Parser.h/cpp`): Builds an Abstract Syntax Tree (AST) from tokens
- **Instruction Encoder** (`InstructionEncoder.h/cpp`): Converts AST to machine code
- **Symbol Table** (`SymbolTable.h`): Manages labels and constants
- **Main Program** (`main.cpp`): Coordinates the process and handles output

## Instruction Encoding Format

The qCore CPU uses a 16-bit instruction format with the following general structure:

```
[15-13] Opcode (3 bits)
[12]    Immediate flag (1 bit)
[11-9]  Register X (3 bits)
[8-0]   Immediate value (9 bits) or special encoding for register instructions
```

For special instructions like XOR, bits 8-0 have specific meanings:

```
[8]     Mode flag (1 for shifts and XOR)
[7]     Immediate/Register mode for shifts
[6-4]   Operation type (for shifts/XOR)
[3-0]   Register Y
```

## Step-by-Step Process

To add a new instruction to the assembler, follow these steps:

### 1. Update Instruction List in Lexer

Add the instruction name to the recognized instructions list in `Lexer.h`:

```cpp
const std::vector<std::string> instructions = {
    // Existing instructions...
    "new_instruction"
};
```

### 2. Define the Instruction Encoding

Add the encoding constant in `InstructionEncoder.h`:

```cpp
static constexpr uint16_t NEW_INSTR = 0xXXXX; // Replace with actual encoding
```

### 3. Update Parser Logic

Modify the `parseInstruction` method in `Parser.cpp` to handle the new instruction's syntax and operands.

### 4. Add Encoding Logic

Implement encoding logic in `InstructionEncoder.cpp`, typically by:
- Adding a case in `encodeInstruction`
- Creating or updating a specialized encoding method if needed

### 5. Update MIF Writer for Disassembly

Modify the `writeMIF` function in `main.cpp` to correctly display the instruction when disassembling.

## XOR Implementation Example

Let's walk through how the XOR instruction is implemented in the qCore assembler:

### 1. Lexer Update

XOR is added to the list of recognized instructions in `Lexer.h`:

```cpp
const std::vector<std::string> instructions = {
    // ...
    "mv", "b", "beq", "bne", "bcc", "bcs", "bpl", "bmi", "bl",
    "mvt", "add", "sub", "ld", "pop", "st", "push", "and", "xor",
    "cmp", "lsl", "lsr", "asr", "ror"
};
```

### 2. Encoding Definition

The XOR instruction's encoding is defined in `InstructionEncoder.h`:

```cpp
static constexpr uint16_t XOR_REG  = 0xE110;
```

This corresponds to the binary pattern `1110XXX100010YYY`, where:
- `111` is opcode 7
- `0XXX` is the destination register (rX)
- `10001` is the XOR operation subtype
- `0YYY` is the source register (rY)

### 3. Parser Logic

In `Parser.cpp`, XOR is handled with other similar instructions:

```cpp
if (opcode == "lsl" || opcode == "lsr" || opcode == "asr" || opcode == "ror" || opcode == "xor") {
    if (check(TokenType::REGISTER)) {
        operand2 = advance().value;
    }
    else if (check(TokenType::NUMBER) || check(TokenType::NUMBER_IMMEDIATE)) {
        if(opcode == "xor") {
            throw std::runtime_error("XOR instruction does not support immediate values...");
        }
        operand2 = advance().value;
        isImmediate = true;
    }
    else {
        throw std::runtime_error("Expected register or immediate value after...");
    }
    return std::make_unique<Instruction>(opcode, operand1, operand2, hasComma, false, isImmediate,
                                       instr.line, instr.column);
}
```

Notice that the parser specifically disallows immediate values for XOR instructions.

### 4. Encoder Implementation

In `InstructionEncoder.cpp`, XOR is handled in the `encodeALUInstruction` method:

```cpp
else if(instr->opcode == "xor") {
    baseOpcode = XOR_REG;
    instr->isImmediate = false;
    context = "xor";
}
```

This sets the proper base opcode and ensures XOR only works with registers, not immediates.

### 5. MIF Writer / Disassembler Update

In `main.cpp`, the `writeMIF` function includes logic to disassemble XOR instructions:

```cpp
case 7:
{
    uint16_t op_subtype = (instr >> 4) & 0x7;
    if (op_subtype == 1) {
        oss << "xor  " << regNames[rX] << ", " << regNames[rY];
    }
    else {
        // Handle other case 7 instructions (shifts, compares)
    }
}
```

This code extracts bits 4-6 to determine if it's an XOR instruction (when the value is 1).

## Testing and Debugging

After adding a new instruction, follow these testing procedures:

1. **Assemble Simple Programs**: Create test programs using your new instruction
2. **Examine the MIF Output**: Verify the correct binary encoding
3. **Compare Disassembly**: Ensure the disassembler correctly identifies your instruction
4. **Test Edge Cases**: Try various operand values, especially boundary conditions
5. **Integration Testing**: Test your instruction in larger programs

### Debugging XOR Example

When working with XOR, a common issue was the decoding pattern. Originally, the code checked for:

```cpp
uint16_t xor_pattern = (instr >> 8) & 0xFF;
if (xor_pattern == 0x11) { // This was incorrect
```

This didn't match the actual encoding because the pattern should be detected using bits 4-6:

```cpp
uint16_t op_subtype = (instr >> 4) & 0x7;
if (op_subtype == 1) { // This correctly identifies XOR instructions
```

This example highlights the importance of ensuring consistency between encoding and decoding.
