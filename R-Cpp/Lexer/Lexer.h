#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "../Util/Token.h"

class Lexer
{
public:
    using iterator = size_t;

    Lexer(const std::string& filename);
    Token nextToken();
    void setIterator(iterator it);
    iterator getIterator();
    Token viewNextToken();
    Token curToken();

private:
    Token nextIdentifier();
    Token nextNumber();
    Token skipComment();
    Token nextCharacter();
    Token getNextToken();
    void getNextChar();
    int getLineNo();
    int getCharNo();
    Token makeToken(TokenType tok,const std::string& content="");

    std::fstream fstream_;
    char lastChar_;
    int lineCount, charCount;
    iterator index_;
    std::vector<Token> tokens_;
};
