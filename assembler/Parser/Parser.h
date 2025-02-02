#pragma once
#include "common.h"
#include "Lexer/Lexer.h"
#include <memory>
#include <vector>
#include <string>

enum class StatementType {
    INSTRUCTION,
    DIRECTIVE,
    LABEL
};

class Statement {
public:
    StatementType type;
    int line;
    int column;

    Statement(StatementType t, int l, int c) : type(t), line(l), column(c) {}
    virtual ~Statement() = default;
};

class Instruction : public Statement {
public:
    std::string opcode;
    std::string operand1;
    std::string operand2;
    bool hasComma;
    bool isLabelImmediate;

    Instruction(const std::string& op, const std::string& op1, 
                const std::string& op2, bool comma, bool labelImm, int l, int c)
        : Statement(StatementType::INSTRUCTION, l, c), 
          opcode(op), operand1(op1), operand2(op2), hasComma(comma), isLabelImmediate(labelImm) {}
};

class Directive : public Statement {
public:
    std::string name;
    std::string label;
    std::string value;

    Directive(const std::string& n, const std::string& l, 
              const std::string& v, int line, int col)
        : Statement(StatementType::DIRECTIVE, line, col), 
          name(n), label(l), value(v) {}
};

class Label : public Statement {
public:
    std::string name;

    Label(const std::string& n, int l, int c)
        : Statement(StatementType::LABEL, l, c), name(n) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;

    Token peek() const;
    Token advance();
    Token previous() const;
    bool isAtEnd() const;
    bool match(TokenType type);
    bool check(TokenType type) const;

    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Instruction> parseInstruction();
    std::unique_ptr<Directive> parseDirective();
    std::unique_ptr<Label> parseLabel();

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}
    std::vector<std::unique_ptr<Statement>> parse();
};