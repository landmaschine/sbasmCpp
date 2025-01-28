#include "common.h"
#include "Lexer/Lexer.h"
#include <iomanip>

int main(int argc, const char* argv[]) {
    std::string input = R"(
        .define LED_ADDRESS 0x1000
        .define HEX_ADDRESS 0x2000
        .define SWI_ADDRESS 0x3000

        // Example: Shift and Add Algorithm
        //     00001011  11
        //   * 00110101  53
        //   ----------
        //         1011
        //       1011
        //     1011
        //   +1011
        //   ----------
        //   1001000111  583

        START:  mv   sp, =0x400

        TEST:   bl   SC
                mv   r0, #11
                mv   r1, #53
                push r0
                push r1
                bl   MUL8x8
                pop  r3             // =583
                bl   RC

        FIN:    .word 0b1110000111110000 // Halt
        //------------------------------------------------
        MUL8x8: mv   r3, #0x00      // Result Register
                pop  r1
                pop  r0

        MLOOP:  mv   r4, r1
                and  r1, #0x01
                cmp  r1, #0x01
                beq  ADDM
                b    MCONT

        ADDM:   add  r3, r0

        MCONT:  lsl  r0, #0x01
                lsr  r4, #0x01
                mv   r1, r4
                cmp  r1, #0x00
                bne  MLOOP
                push r3             // Push Result
                mv   pc, lr
        //------------------------------------------------
        SC:     push r0             // Save Context
                push r1
                push r2
                push r3
                push r4
                mv   pc, lr

        RC:     pop  r4             // Restore Context
                pop  r3
                pop  r2
                pop  r1
                pop  r0
                mv   pc, lr

    )";

    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();

    size_t max_type_width = 0;
    size_t max_value_width = 0;
    size_t max_line_width = 0;

    for (const auto& token : tokens) {
        max_type_width = std::max(max_type_width, std::to_string(static_cast<int>(token.type)).length());
        max_value_width = std::max(max_value_width, token.value.length());
        max_line_width = std::max(max_line_width, std::to_string(token.line).length());
    }
    
    for (const auto& token : tokens) {
        std::cout << "Type: "   << std::setw(max_type_width)  << static_cast<int>(token.type)
                << " | Value: " << std::setw(max_value_width) << token.value 
                << " | Line: "  << std::setw(max_line_width)  << token.line 
                << " | Column: " << token.column << std::endl;
    }

    return 0;
};
