#include "Lexer.h"

void Lexer::skipWhiteSpace() {
    while (position < input.length() && std::isspace(input[position])) {
        // Window whitespace 
        if (input[position] == '\r' && position + 1 < input.length() && input[position + 1] == '\n') {
            line++;
            column = 1;
            position += 2;
        // Unix whitespace
        } else if (input[position] == '\n') {
            line++;
            column = 1;
            position++;
        } else {
            column++;
            position++;
        }
    }
}

Token Lexer::parseNumber() {
   std::string number;
    int start_column = column;

    if (input[position] == '#' || input[position] == '=') {
        number += input[position];
        position++;
        column++;
    }

    if (position < input.length() && input[position] == '-') {
        number += input[position];
        position++;
        column++;
    }

    if (position < input.length() && (std::isalpha(input[position]) || input[position] == '_')) {
        while (position < input.length() && isIdentifierChar(input[position])) {
            number += input[position];
            position++;
            column++;
        }
    } else {
        if (position < input.length() && input[position] == '0') {
            number += input[position];
            position++;
            column++;
            if (position < input.length() && (input[position] == 'x' || input[position] == 'b')) {
                number += input[position];
                position++;
                column++;
            }
        }

        while (position < input.length() && (std::isxdigit(input[position]) || input[position] == '-')) {
            number += input[position];
            position++;
            column++;
        }
    }

    return Token(TokenType::NUMBER, number, line, start_column); 
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

    if (identifier[0] == 'r' && identifier.length() == 2 && std::isdigit(identifier[1]) && (identifier[1] - '0') <= 7) {
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

Token Lexer::nextToken() {
    if(position >= input.length()) {
        return Token(TokenType::END_OF_FILE, "END_OF_FILE", line, column);
    }

    skipWhiteSpace();

    if(position >= input.length()) {
        return Token(TokenType::END_OF_FILE, "END_OF_FILE", line, column);
    }

    char current = input[position];
    int start_column = column;

    switch(current) {
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
        case '#':
        case '=':
            return parseNumber();
        case '/':
            if(position + 1 < input.length() && input[position + 1] == '/') {
                std::string comment;
                while(position < input.length() && input[position] != '\n') {
                    comment += input[position];
                    position++;
                    column++;
                }
                if(position < input.length() && input[position] == '\n') {
                    position++;
                    line++;
                    column = 1;
                }
                return Token(TokenType::COMMENT, comment, line, start_column);
            }
            position++;
            column++;
            return Token(TokenType::INVALID, std::string(1, current), line, start_column);
    }

    if(std::isdigit(current) || (current == '-' && position + 1 < input.length() && std::isdigit(input[position + 1]))) {
        return parseNumber();
    }

    if(std::isalpha(current) || current == '.' || current == '_' || current == '$') {
        return parseIdentifier();
    }

    position++;
    column++;
    return Token(TokenType::INVALID, std::string(1, current), line, start_column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();

    while(token.type != TokenType::END_OF_FILE) {
        if(token.type != TokenType::INVALID) {
            tokens.push_back(token);
        }
        token = nextToken();
    }

    tokens.push_back(token);

    return tokens;
}
