#pragma once
#include <string>
#include "llvm/IR/Instructions.h"

struct Variable
{
    Variable()
        :val(nullptr)
    { }

    Variable(const std::string& Name, const std::string& Type, llvm::AllocaInst* Val)
        :type(Type),name(Name),val(Val)
    { }

    std::string type;
    std::string name;
    llvm::AllocaInst* val;
};