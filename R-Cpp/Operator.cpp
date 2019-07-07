#include "Operator.h"
#include <map>
OperatorType TokenToBinOperator(TokenType t) {
    switch (t) {
    case TokenType::Point:
        return OperatorType::MemberAccessP;
    case TokenType::Exclam:
        return OperatorType::LogicalNOT;
    case TokenType::Multiply:
        return OperatorType::Multiplication;
    case TokenType::Divide:
        return OperatorType::Division;
    case TokenType::Percent:
        return OperatorType::Remainder;
    case TokenType::Plus:
        return OperatorType::Addition;
    case TokenType::Minus:
        return OperatorType::Subtraction;
    case TokenType::lAngle:
        return OperatorType::Less;
    case TokenType::rAngle:
        return OperatorType::Greater;
    case TokenType::And:
        return OperatorType::BitwiseAND;
    case TokenType::Or:
        return OperatorType::BitwiseOR;
    case TokenType::Equal:
        return OperatorType::Assignment;
    case TokenType::Comma:
        return OperatorType::Comma;
    default:
        return OperatorType::None;
    }
}

int getBinOperatorPrecedence(OperatorType t)
{
    static const std::map<OperatorType, int> BinopPrecedence
    {
        {OperatorType::Assignment, 2},
        {OperatorType::SumComAssign, 2},
        {OperatorType::MinComAssign, 2},
        {OperatorType::MulComAssign, 2},
        {OperatorType::DivComAssign, 2},
        {OperatorType::RemComAssign, 2},
        {OperatorType::LshComAssign, 2},
        {OperatorType::RshComAssign, 2},
        {OperatorType::ANDComAssign, 2},
        {OperatorType::XORComAssign, 2},
        {OperatorType::ORComAssign, 2},
        {OperatorType::LogicalOR, 3},
        {OperatorType::LogicalAND, 4},
        {OperatorType::BitwiseOR, 5},
        {OperatorType::BitwiseXOR, 6},
        {OperatorType::BitwiseAND, 7},
        {OperatorType::Equal, 8},
        {OperatorType::NotEqual, 8},
        {OperatorType::Less, 10},
        {OperatorType::LessEqual, 10},
        {OperatorType::Greater, 10},
        {OperatorType::GreaterEqual, 10},
        {OperatorType::LeftShift, 15},
        {OperatorType::RightShift, 15},
        {OperatorType::Addition, 20},
        {OperatorType::Subtraction, 20},
        {OperatorType::Multiplication, 40},
        {OperatorType::Division, 40},
        {OperatorType::Remainder, 40},
        {OperatorType::MemberAccessP, 60},
        {OperatorType::MemberAccessA, 60},
        {OperatorType::ScopeResolution, 100}
    };
    auto it = BinopPrecedence.find(t);
    if (it == BinopPrecedence.end()) return -1;
    return it->second;
}
