#pragma once
#include "Token.h"
#include <llvm/IR/Value.h>

namespace CG {
    class CodeGenerator;
}

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
    Promotion,        // +a
    Negation,         // -a
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

bool isCompoundAssignOperator(OperatorType t);
bool isCompareOperator(OperatorType t);
OperatorType compoundAssignToOperator(OperatorType t);

llvm::Value* builtinTypeOperate(llvm::Value* LHS, const std::string& ltype,
    llvm::Value* RHS, const std::string& rtype, OperatorType op
    , CG::CodeGenerator& cg);
std::string builtinOperatorReturnType(const std::string& ltype, const std::string& rtype, OperatorType op);

bool isBinaryOperator(OperatorType t);
bool isUnaryOperator(OperatorType t);
