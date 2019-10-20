#include "Operator.h"
#include "../CodeGenerator/CodeGenerator.h"
#include <map>
#include <iostream>
#include <llvm/IR/Constants.h>

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

int getBinOperatorPrecedence(OperatorType t) {
    static const std::map<OperatorType, int> BinopPrecedence
    {
        {OperatorType::None, 0},
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

bool isCompoundAssignOperator(OperatorType t) {
    switch (t) {
    case OperatorType::SumComAssign:
    case OperatorType::MinComAssign:
    case OperatorType::DivComAssign:
    case OperatorType::RemComAssign:
    case OperatorType::LshComAssign:
    case OperatorType::RshComAssign:
    case OperatorType::ANDComAssign:
    case OperatorType::XORComAssign:
    case OperatorType::ORComAssign:
        return true;
    default:
        return false;
    }
}

OperatorType compoundAssignToOperator(OperatorType t)
{
    switch (t) {
    case OperatorType::SumComAssign:
        return OperatorType::Addition;
    case OperatorType::MinComAssign:
        return OperatorType::Subtraction;
    case OperatorType::DivComAssign:
        return OperatorType::Division;
    case OperatorType::RemComAssign:
        return OperatorType::Remainder;
    case OperatorType::LshComAssign:
        return OperatorType::LeftShift;
    case OperatorType::RshComAssign:
        return OperatorType::RightShift;
    case OperatorType::ANDComAssign:
        return OperatorType::BitwiseAND;
    case OperatorType::XORComAssign:
        return OperatorType::BitwiseXOR;
    case OperatorType::ORComAssign:
        return OperatorType::BitwiseOR;
    default:
        return OperatorType::None;
    }
}


bool isCompareOperator(OperatorType t)
{
    switch (t) {
    case OperatorType::Less:
    case OperatorType::LessEqual:
    case OperatorType::Greater:
    case OperatorType::GreaterEqual:
    case OperatorType::Equal:
    case OperatorType::NotEqual:
        return true;
    default:
        return false;
    }
}


llvm::Value* builtinTypeOperator_float(llvm::Value* LHS, llvm::Value* RHS, OperatorType op, CG::CodeGenerator& cg) {
    llvm::Value* tmp = nullptr;
    auto& builder = cg.builder();
    switch (op) {
    case OperatorType::Multiplication:
        return builder.CreateFMul(LHS, RHS);
    case OperatorType::Division:
        return builder.CreateFDiv(LHS, RHS);
    case OperatorType::Addition:
        return builder.CreateFAdd(LHS, RHS);
    case OperatorType::Subtraction:
        return builder.CreateFSub(LHS, RHS);
    case OperatorType::Less:
        return builder.CreateFCmpULT(LHS, RHS);
    case OperatorType::LessEqual:
        return builder.CreateFCmpULE(LHS, RHS);
    case OperatorType::Greater:
        return builder.CreateFCmpUGE(LHS, RHS);
    case OperatorType::GreaterEqual:
        return builder.CreateFCmpUGE(LHS, RHS);
    case OperatorType::Equal:
        return builder.CreateFCmpUEQ(LHS, RHS);
    case OperatorType::NotEqual:
        return builder.CreateFCmpUNE(LHS, RHS);
    default:
        return nullptr;
    }

}

llvm::Value* builtinTypeOperator_i32(llvm::Value* LHS, llvm::Value* RHS, OperatorType op, CG::CodeGenerator& cg) {
    auto& builder = cg.builder();
    if (llvm::isa<llvm::ConstantInt>(LHS)) {
        LHS = llvm::ConstantInt::get(LHS->getType(),
            llvm::APInt(RHS->getType()->getIntegerBitWidth(),
                static_cast<llvm::ConstantInt*>(LHS)->getSExtValue(), true));
    }
    if (llvm::isa<llvm::ConstantInt>(RHS)) {
        RHS = llvm::ConstantInt::get(RHS->getType(),
            llvm::APInt(LHS->getType()->getIntegerBitWidth(),
                static_cast<llvm::ConstantInt*>(RHS)->getSExtValue(), true));
    }
    switch (op) {
    case OperatorType::Multiplication:
        return builder.CreateMul(LHS, RHS);
    case OperatorType::Division:
        return builder.CreateSDiv(LHS, RHS);
    case OperatorType::Remainder:
        return builder.CreateSRem(LHS, RHS);
    case OperatorType::Addition:
        return builder.CreateAdd(LHS, RHS);
    case OperatorType::Subtraction:
        return builder.CreateSub(LHS, RHS);
    case OperatorType::LeftShift:
        return builder.CreateShl(LHS, RHS);
    case OperatorType::RightShift:
        return builder.CreateAShr(LHS, RHS);
    case OperatorType::Less:
        return builder.CreateICmpSLT(LHS, RHS);
    case OperatorType::LessEqual:
        return builder.CreateICmpSLE(LHS, RHS);
    case OperatorType::Greater:
        return builder.CreateICmpSGE(LHS, RHS);
    case OperatorType::GreaterEqual:
        return builder.CreateICmpSGE(LHS, RHS);
    case OperatorType::Equal:
        return builder.CreateICmpEQ(LHS, RHS);
    case OperatorType::NotEqual:
        return builder.CreateICmpNE(LHS, RHS);
    case OperatorType::BitwiseAND:
    case OperatorType::LogicalAND:
        return builder.CreateAnd(LHS, RHS);
    case OperatorType::BitwiseOR:
    case OperatorType::LogicalOR:
        return builder.CreateOr(LHS, RHS);
    default:
        return nullptr;
    }
}

llvm::Value* builtinTypeOperate(llvm::Value* LHS, const std::string& ltype,
    llvm::Value* RHS, const std::string& rtype, OperatorType op
    , CG::CodeGenerator& cg) {
    if (ltype == "i32") {
        if (rtype == "i32") return builtinTypeOperator_i32(LHS, RHS, op, cg);
    }
    else if(ltype=="float"||ltype=="double")
    {
        if (rtype == "float" || rtype == "double") return builtinTypeOperator_float(LHS, RHS, op, cg);
    }
    std::cout << "No suitable operator between " << ltype
        << " and " << rtype << "." << std::endl;
    return nullptr;
}

std::string builtinOperatorReturnType(const std::string& ltype,const std::string& rtype,OperatorType op)
{
    if (isCompareOperator(op)) return "bool";
    if (isCompoundAssignOperator(op)) return ltype;
    if (ltype == "float" || rtype == "float") return "float";
    return "i32";
}

bool isBinaryOperator(OperatorType t)
{
    if (isUnaryOperator(t)) return false;
    if (t == OperatorType::Comma) return false;
    return true;
}

bool isUnaryOperator(OperatorType t)
{
    return t == OperatorType::PostIncrement || t == OperatorType::PostDecrement ||
        t == OperatorType::FunctionCall || t == OperatorType::Subscript ||
        t == OperatorType::MemberAccessP || t == OperatorType::MemberAccessA ||
        t == OperatorType::LogicalNOT || t == OperatorType::BitwiseNOT ||
        t == OperatorType::Dereference;
}

std::ostream& operator<<(std::ostream& os, OperatorType op) {
    switch (op) {
    case OperatorType::ScopeResolution:
        os << "ScopeResolution ::";
        break;
    case OperatorType::PostIncrement:
        os << "PostIncrement ++";
        break;
    case OperatorType::PostDecrement:
        os << "PostDecrement --";
        break;
    case OperatorType::FunctionCall:
        os << "FunctionCall ()";
        break;
    case OperatorType::Subscript:
        os << "Subscript []";
        break;
    case OperatorType::MemberAccessA:
        os << "MemberAccess ->";
        break;
    case OperatorType::MemberAccessP:
        os << "MemberAccess .";
        break;
    case OperatorType::PreIncrement:
        os << "PreIncrement ++";
        break;
    case OperatorType::PreDecrement:
        os << "PreDecrement --";
        break;
    case OperatorType::Promotion:
        os << "Promotion +";
        break;
    case OperatorType::Negation:
        os << "Negation -";
        break;
    case OperatorType::LogicalNOT:
        os << "LogicalNot !";
        break;
    case OperatorType::BitwiseNOT:
        os << "BitwiseNOT ~";
        break;
    case OperatorType::Dereference:
        os << "Dereference *";
        break;
    case OperatorType::Multiplication:
        os << "Multiplication *";
        break;
    case OperatorType::Division:
        os << "Division /";
        break;
    case OperatorType::Remainder:
        os << "Remainder %";
        break;
    case OperatorType::Addition:
        os << "Addition +";
        break;
    case OperatorType::Subtraction:
        os << "Subtraction -";
        break;
    case OperatorType::LeftShift:
        os << "LeftShift <<";
        break;
    case OperatorType::RightShift:
        os << "RightShift >>";
        break;
    case OperatorType::Less:
        os << "Less <";
        break;
    case OperatorType::LessEqual:
        os << "LessEqual <=";;
        break;
    case OperatorType::Greater:
        os << "Greater >";
        break;
    case OperatorType::GreaterEqual:
        os << "GreaterEqual >=";
        break;
    case OperatorType::Equal:
        os << "Equal ==";
        break;
    case OperatorType::NotEqual:
        os << "NotEqual !=";
        break;
    case OperatorType::BitwiseAND:
        os << "BitwiseAND &";
        break;
    case OperatorType::BitwiseXOR:
        os << "BitwiseXOR ^";
        break;
    case OperatorType::BitwiseOR:
        os << "BitwiseOR |";
        break;
    case OperatorType::LogicalAND:
        os << "LogicalAND &&";
        break;
    case OperatorType::LogicalOR:
        os << "LogicalOR ||";
        break;
    case OperatorType::Assignment:
        os << "Assignment =";
        break;
    case OperatorType::SumComAssign:
        os << "SumCompoundAssignment +=";
        break;
    case OperatorType::MinComAssign:
        os << "MinusCompoundAssignment -=";
        break;
    case OperatorType::MulComAssign:
        os << "MultiplyCompoundAssignment *=";
        break;
    case OperatorType::DivComAssign:
        os << "DivisionCompoundAssignment /=";
        break;
    case OperatorType::RemComAssign:
        os << "RemainderCompoundAssignment %=";
        break;
    case OperatorType::LshComAssign:
        os << "LeftShiftCompoundAssignment <<=";
        break;
    case OperatorType::RshComAssign:
        os << "RightShiftCompoundAssignment >>=";
        break;
    case OperatorType::ANDComAssign:
        os << "ANDCompoundAssignment &=";
        break;
    case OperatorType::XORComAssign:
        os << "XORCompoundAssignment ^=";
        break;
    case OperatorType::ORComAssign:
        os << "ORCompoundAssignment |=";
        break;
    case OperatorType::Comma:
        os << "Comma ,";
        break;
    default:
        os << "<Unknown Operator>";
    }
    return os;
}
