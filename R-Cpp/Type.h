#pragma once
#include <string>
#include <llvm/IR/Type.h>
#include "CodeGenerator.h"

//enum class Type
//{
//    i32,i64,u32,u64,Bool,Float,Double,none
//};
//
llvm::Type* get_builtin_type(const std::string& s,CodeGenerator& cg) {
    if (s == "i32") return llvm::Type::getInt32Ty(cg.context());
    if (s == "i64") return llvm::Type::getInt64Ty(cg.context());
    if (s == "u32") return llvm::Type::getInt32Ty(cg.context());
    if (s == "u64") return llvm::Type::getInt64Ty(cg.context());
    if (s == "bool") return llvm::Type::getInt1Ty(cg.context());
    if (s == "float") return llvm::Type::getFloatTy(cg.context());
    if (s == "double") return llvm::Type::getDoubleTy(cg.context());
    return nullptr;
}

llvm::Value* get_builtin_type_default_value(const std::string& s, CodeGenerator& cg) {
    if (s == "i32") return llvm::ConstantInt::get(cg.context(),llvm::APInt(32,0,true));
    if (s == "i64") return llvm::ConstantInt::get(cg.context(), llvm::APInt(64, 0, true));
    if (s == "u32") return llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, false));
    if (s == "u64") return llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, false));
    if (s == "bool") return llvm::ConstantInt::get(cg.context(), llvm::APInt(1, 0, true));
    if (s == "float") return llvm::ConstantFP::get(cg.context(), llvm::APFloat(0.0));
    if (s == "double") return llvm::ConstantFP::get(cg.context(), llvm::APFloat(0.0));
    return nullptr;
}