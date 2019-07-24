#pragma once
#include <fstream>
#include <string>
#include "Token.h"

class Lexer
{
public:
    Lexer(const std::string& filename);
    Token nextToken();
    int getLineNo();
    int getCharNo();
    char nextChar();

private:
    Token nextIdentifier();
    Token nextNumber();
    Token skipComment();
    Token nextCharacter();
    void getNextChar();

    std::fstream fstream_;
    char lastChar_;
    int lineCount, charCount;
};