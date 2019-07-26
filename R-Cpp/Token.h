#pragma once
#include <string>

enum class TokenType
{
    Function,
    Import,
    Trait,
    Export,
    Namespace,
    Class,
    Return,
    If,
    Else,
    For,
    Identifier,
    Integer,
    Float,
    External,
    Internal,
    lParenthesis = '(',
    rParenthesis = ')',
    lSquare = '[',
    rSquare = ']',
    lBrace = '{',
    rBrace = '}',
    lAngle='<',
    rAngle='>',
    Colon=':',
    Semicolon=';',
    Plus='+',
    Minus='-',
    Multiply='*',
    Divide='/',
    Comma=',',
    Point='.',
    Equal='=',
    And='&',
    Or='|',
    Xor='^',
    Exclam='!',
    At='@',
    Pound='#',
    Dollar='$',
    Percent='%',
    SingleQuote='\'',
    DoubleQuote='"',
    Question='?',
    Tilde= '~',
    Unknown,
    Eof=EOF
};

struct TokenHash
{
    int operator()(TokenType t) {
        return std::hash<int>{}(static_cast<int>(t));
    }
};

struct Token
{
    Token(TokenType t, std::string s) :type(t), content(s) {}
    Token(TokenType t) :type(t), content() {}

    TokenType type;
    std::string content;
};

constexpr TokenType charToToken(char c)
{
    switch (c) {
    case '(':
        return TokenType::lParenthesis;
    case ')':
        return TokenType::rParenthesis;
    case '[':
        return TokenType::lSquare;
    case ']':
        return TokenType::rSquare;
    case '{':
        return TokenType::lBrace;
    case '}':
        return TokenType::rBrace;
    case '<':
        return TokenType::lAngle;
    case '>':
        return TokenType::rAngle;
    case ':':
        return TokenType::Colon;
    case ';':
        return TokenType::Semicolon;
    case '+':
        return TokenType::Plus;
    case '*':
        return TokenType::Multiply;
    case '-':
        return TokenType::Minus;
    case '/':
        return TokenType::Divide;
    case ',':
        return TokenType::Comma;
    case '.':
        return TokenType::Point;
    case'=':
        return TokenType::Equal;
    case'&':
        return TokenType::And;
    case '|':
        return TokenType::Or;
    case '^':
        return TokenType::Xor;
    case'!':
        return TokenType::Exclam;
    case '@':
        return TokenType::At;
    case '#':
        return TokenType::Pound;
    case '$':
        return TokenType::Dollar;
    case '%':
        return TokenType::Percent;
    case '\'':
        return TokenType::SingleQuote;
    case '"':
        return TokenType::DoubleQuote;
    case '?':
        return TokenType::Question;
    default:
        return TokenType::Unknown;
    }
}