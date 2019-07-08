#pragma once
#include "Token.h"

enum class OperatorType
{
    ScopeResolution,  // ::
    PostIncrement,    // a++
    PostDecrement,    // a--
    FunctionCall,     // a()
    Subscript,        // a[]
    MemberAccessP,    // . 
    MemberAccessA,    // ->
    PreIncrement,     // ++a
    PreDecrement,     // --a
    LogicalNOT,       // !
    BitwiseNOT,       // ~
    Dereference,      // *
    Multiplication,   // a*b
    Division,         // a/b
    Remainder,        // a%b
    Addition,         // a+b
    Subtraction,      // a-b
    LeftShift,        // <<
    RightShift,       // >>
    Less,             // <
    LessEqual,        // <=
    Greater,          // >
    GreaterEqual,     // >=
    Equal,            // ==
    NotEqual,         // !=
    BitwiseAND,       // &
    BitwiseXOR,       // ^
    BitwiseOR,        // |
    LogicalAND,       // &&
    LogicalOR,        // ||
    Assignment,       // =
    SumComAssign,     // +=
    MinComAssign,     // -=
    MulComAssign,     // *=
    DivComAssign,     // /=
    RemComAssign,     // %=
    LshComAssign,     // <<=
    RshComAssign,     // >>=
    ANDComAssign,     // &=
    XORComAssign,     // ^=
    ORComAssign,      // |=
    Comma,            // ,
    None
};

OperatorType TokenToBinOperator(TokenType t);

int getBinOperatorPrecedence(OperatorType t);

bool isAssignOperator(OperatorType t);
