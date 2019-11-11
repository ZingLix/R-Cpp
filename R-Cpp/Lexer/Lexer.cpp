#include "Lexer.h"
using namespace std;

Lexer::Lexer(const string& filename)
    : fstream_(filename, fstream_.in), lastChar_(' '),
    lineCount(1),charCount(1),index_(0)
{
    if (!fstream_.is_open())
        throw std::runtime_error("Failed to open " + filename + ".");
    while (!fstream_.eof())
    {
        tokens_.push_back(getNextToken());
    }
}

Token Lexer::nextToken()
{
    return tokens_[index_++];
}

void Lexer::setIterator(iterator it)
{
    index_ = it;
}

Lexer::iterator Lexer::getIterator()
{
    return index_;
}

struct Token Lexer::curToken()
{
    return tokens_[index_];
}

Token Lexer::getNextToken()
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
        return makeToken(TokenType::Eof);
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

Token Lexer::viewNextToken()
{
    return tokens_[index_+1];
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
        return makeToken(TokenType::Function);
    if (content == "import")
        return makeToken(TokenType::Import);
    if (content == "trait")
        return makeToken(TokenType::Trait);
    if (content == "namespace")
        return makeToken(TokenType::Namespace);
    if (content == "class")
        return makeToken(TokenType::Class);
    if (content == "return")
        return makeToken(TokenType::Return);
    if (content == "if")
        return makeToken(TokenType::If);
    if (content == "else")
        return makeToken(TokenType::Else);
    if (content == "for")
        return makeToken(TokenType::For);
    if (content == "external")
        return makeToken(TokenType::External);
    if (content == "internal")
        return makeToken(TokenType::Internal);
    if (content == "using")
        return makeToken(TokenType::Using);
    return makeToken(TokenType::Identifier, content);
}

Token Lexer::nextNumber()
{
    // Number: [0-9.]+
    bool isFloat = lastChar_ == '.';
    if(isFloat&&!isdigit(fstream_.peek()))
    {
        // like the point in obj.func()
        getNextChar();
        return makeToken(TokenType::Point);
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
    return makeToken(isFloat ? TokenType::Float : TokenType::Integer, num);
}

Token Lexer::skipComment()
{
    // comment: //...
    while (lastChar_ != '\n' && lastChar_ != '\r')
    {
        getNextChar();
        if (lastChar_ == EOF) return makeToken(TokenType::Eof);
    }
    return nextToken();
}

Token Lexer::nextCharacter()
{
    auto tmp = lastChar_;
    getNextChar();
    return makeToken(charToToken(tmp));
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

Token Lexer::makeToken(TokenType tok, const std::string& content)
{
    return Token(tok, content, getLineNo(), getCharNo());
}
