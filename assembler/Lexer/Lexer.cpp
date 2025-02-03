#include "Lexer.h"

void Lexer::skipWhitespace() {
    while (position < input.length() && std::isspace(input[position])) {
        if (input[position] == '\n') {
            line++;
            column = 1;
            position++;
        } else if (input[position] == '\r' && 
                   position + 1 < input.length() && 
                   input[position + 1] == '\n') {
            line++;
            column = 1;
            position += 2;
        } else {
            column++;
            position++;
        }
    }
}

int64_t Lexer::parseNumberValue(const std::string& str) {
    std::string numStr = str;
    bool isNegative = false;
    
    if (numStr[0] == '#' || numStr[0] == '=') {
        numStr = numStr.substr(1);
    }
    
    if (numStr[0] == '-') {
        isNegative = true;
        numStr = numStr.substr(1);
    }

    int64_t value = 0;
    if (numStr.substr(0, 2) == "0x" || numStr.substr(0, 2) == "0X") {
        value = std::stoll(numStr.substr(2), nullptr, 16);
    } else if (numStr.substr(0, 2) == "0b" || numStr.substr(0, 2) == "0B") {
        value = std::stoll(numStr.substr(2), nullptr, 2);
    } else {
        value = std::stoll(numStr, nullptr, 10);
    }

    return isNegative ? -value : value;
}

Token Lexer::nextToken() {
    if (position >= input.length()) {
        return Token(TokenType::END_OF_FILE, "EOF", line, column);
    }

    skipWhitespace();

    if (position >= input.length()) {
        return Token(TokenType::END_OF_FILE, "EOF", line, column);
    }

    char current = input[position];
    int start_column = column;
    
    if (current == '#' || current == '=') {
        bool isEquals = (current == '=');
        position++;
        column++;
        
        skipWhitespace();
        
        std::string value;
        
        if (position < input.length() && input[position] == '-') {
            value += input[position];
            position++;
            column++;
        }

        if (position < input.length()) {
            if (std::isdigit(input[position])) {
                while (position < input.length() && 
                       (std::isdigit(input[position]) || 
                        input[position] == 'x' || input[position] == 'X' ||
                        input[position] == 'b' || input[position] == 'B' ||
                        std::isxdigit(input[position]))) {
                    value += input[position];
                    position++;
                    column++;
                }
                return Token(isEquals ? TokenType::LABEL_IMMEDIATE : TokenType::NUMBER_IMMEDIATE, 
                           value, line, start_column);
            }
            else if (std::isalpha(input[position]) || input[position] == '_' || input[position] == '$') {
                while (position < input.length() && 
                       (std::isalnum(input[position]) || input[position] == '_' || input[position] == '$')) {
                    value += input[position];
                    position++;
                    column++;
                }
                return Token(isEquals ? TokenType::LABEL_IMMEDIATE : TokenType::NUMBER_IMMEDIATE, value, line, start_column);
            }
        }
        throw std::runtime_error("Invalid immediate value at line " + std::to_string(line) + ", column " + std::to_string(start_column));
    }

    switch (current) {
        case ',':
            position++;
            column++;
            return Token(TokenType::COMMA, ",", line, start_column);
        case '[':
            position++;
            column++;
            return Token(TokenType::BRACKET_OPEN, "[", line, start_column);
        case ']':
            position++;
            column++;
            return Token(TokenType::BRACKET_CLOSE, "]", line, start_column);
        case '/':
            if (position + 1 < input.length() && input[position + 1] == '/') {
                while (position < input.length() && input[position] != '\n') {
                    position++;
                    column++;
                }
                return nextToken();
            }
            break;
    }

    if (std::isdigit(current) || current == '-') {
        std::string number;
        if (current == '-') {
            number += current;
            position++;
            column++;
            if (position >= input.length() || !std::isdigit(input[position])) {
                return Token(TokenType::INVALID, number, line, start_column);
            }
        }

        while (position < input.length() && 
               (std::isdigit(input[position]) || 
                input[position] == 'x' || input[position] == 'X' ||
                input[position] == 'b' || input[position] == 'B' ||
                std::isxdigit(input[position]))) {
            number += input[position];
            position++;
            column++;
        }
        return Token(TokenType::NUMBER, number, line, start_column);
    }
    
    if (std::isalpha(current) || current == '.' || current == '_' || current == '$') {
        return parseIdentifier();
    }

    position++;
    column++;
    return Token(TokenType::INVALID, std::string(1, current), line, start_column);
}

Token Lexer::parseIdentifier() {
    std::string identifier;
    int start_column = column;

    if (input[position] == '.') {
        identifier += input[position];
        position++;
        column++;
    }

    while (position < input.length() && isIdentifierChar(input[position])) {
        identifier += input[position];
        position++;
        column++;
    }
    
    if (position < input.length() && input[position] == ':') {
        position++;
        column++;
        return Token(TokenType::LABEL, identifier, line, start_column);
    }
    
    if (identifier[0] == '.' && (identifier == ".word" || identifier == ".define")) {
        return Token(TokenType::DIRECTIVE, identifier, line, start_column);
    }
    
    if (identifier[0] == 'r' && identifier.length() == 2 && 
        std::isdigit(identifier[1]) && (identifier[1] - '0') <= 7) {
        return Token(TokenType::REGISTER, identifier, line, start_column);
    }
    
    if (identifier == "sp" || identifier == "lr" || identifier == "pc") {
        return Token(TokenType::REGISTER, identifier, line, start_column);
    }
    
    if (isInstruction(identifier)) {
        return Token(TokenType::INSTRUCTION, identifier, line, start_column);
    }

    return Token(TokenType::LABEL_REF, identifier, line, start_column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();

    while (token.type != TokenType::END_OF_FILE) {
        if (token.type != TokenType::INVALID) {
            tokens.push_back(token);
        }
        token = nextToken();
    }

    tokens.push_back(token);
    return tokens;
}