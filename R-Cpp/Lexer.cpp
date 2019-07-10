#include "Lexer.h"
using namespace std;

Lexer::Lexer(const string& filename)
    : fstream_(filename, fstream_.in), lastChar_(' '),
    lineCount(1),charCount(1)
{
    if (!fstream_.is_open())
        throw std::runtime_error("Failed to open " + filename + ".");
}

Token Lexer::nextToken()
{
    while (isspace(lastChar_)||lastChar_=='\n'||lastChar_=='\r')
        getNextChar();
    if (isalpha(lastChar_)||lastChar_=='_')
        return nextIdentifier();
    if (isdigit(lastChar_) || lastChar_ == '.')
        return nextNumber();
    if (lastChar_ == '/')
    {
        if (fstream_.peek() == '/')
        {
            return skipComment();
        }
    }
    if (lastChar_ == EOF)
        return Token(TokenType::Eof);
    return nextCharacter();
}

int Lexer::getLineNo()
{
    return lineCount;
}

int Lexer::getCharNo()
{
    return charCount;
}

Token Lexer::nextIdentifier()
{
    // identifier: [a-zA-Z][a-zA-Z0-9]*
    string content;
    while (isalnum(lastChar_)||lastChar_=='_')
    {
        content += lastChar_;
        getNextChar();
    }
    if (content == "fn")
        return Token(TokenType::Function);
    if (content == "import")
        return Token(TokenType::Import);
    if (content == "trait")
        return Token(TokenType::Trait);
    if (content == "namespace")
        return Token(TokenType::Namespace);
    if (content == "class")
        return Token(TokenType::Class);
    if (content == "return")
        return Token(TokenType::Return);
    if (content == "if")
        return Token(TokenType::If);
    if (content == "else")
        return Token(TokenType::Else);
    if (content == "for")
        return Token(TokenType::For);
    return Token(TokenType::Identifier, content);
}

Token Lexer::nextNumber()
{
    // Number: [0-9.]+
    bool isFloat = lastChar_ == '.';
    if(isFloat&&!isdigit(fstream_.peek()))
    {
        // like the point in obj.func()
        getNextChar();
        return Token(TokenType::Point);
    }
    string num;
    while (isdigit(lastChar_) || lastChar_ == '.')
    {
        if (lastChar_ == '.')
        {
            if (isFloat) throw std::invalid_argument("Bad number.");
            isFloat = true;
        }
        num += lastChar_;
        getNextChar();
    }
    return Token(isFloat ? TokenType::Float : TokenType::Integer, num);
}

Token Lexer::skipComment()
{
    // comment: //...
    while (lastChar_ != '\n' && lastChar_ != '\r')
    {
        getNextChar();
        if (lastChar_ == EOF) return Token(TokenType::Eof);
    }
    return nextToken();
}

Token Lexer::nextCharacter()
{
    auto tmp = lastChar_;
    getNextChar();
    return Token(charToToken(tmp));
}

void Lexer::getNextChar()
{
    lastChar_ = fstream_.get();
    if(lastChar_=='\n')
    {
        lineCount++;
        charCount = 1;
    }else
    {
        charCount++;
    }
}
