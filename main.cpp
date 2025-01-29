#include "common.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <iomanip>

int main(int argc, const char* argv[]) {
    std::string input = R"(
        .define LED_ADDRESS 0x1000
        .define HEX_ADDRESS 0x2000
        .define SWI_ADDRESS 0x3000

        START: mv r1, #52

    )";

    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::vector<std::unique_ptr<Statement>> ast = parser.parse();

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

    std::cout << "\n\n";

    for(const auto& stmt : ast) {
        switch(stmt->type) {
            case StatementType::INSTRUCTION:

                break;
            case StatementType::DIRECTIVE:
            
                break;
            case StatementType::LABEL:

                break;
        }
    }

    return 0;
};
