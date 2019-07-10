#pragma once
#include <memory>
#include <map>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "Type.h"
#include "Operator.h"
#include "Type.h"
#include "SymbolTable.h"
#include <iostream>

class CodeGenerator
{
public:
    CodeGenerator();

    llvm::LLVMContext& context();
    llvm::IRBuilder<>& builder();
    llvm::Function* getFunction(const std::string& Callee);
    llvm::Module& getModule();
    llvm::legacy::FunctionPassManager* FPM();

    void output();

    SymbolTable& symbol();

private:
    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    std::shared_ptr<SymbolTable> Symbol;
};

