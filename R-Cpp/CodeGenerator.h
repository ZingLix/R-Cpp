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


class CodeGenerator
{
public:
    CodeGenerator()
        :Builder(TheContext),TheModule(std::make_unique<llvm::Module>("RCpp",context()))
    {}

    llvm::LLVMContext& context() {
        return TheContext;
    }

    llvm::Value* getValue(const std::string& name) {
        return NamedValues[name];
    }

    void setValue(const std::string& name, llvm::Value* val) {
        NamedValues[name] = val;
    }

    void clearValue() {
        NamedValues.clear();
    }

    llvm::IRBuilder<>& builder() {
        return Builder;
    }

    llvm::Function* getFunction(const std::string& Callee) {
        return TheModule->getFunction(Callee);
    }

    llvm::Module& getModule() {
        return *TheModule;
    }

private:
    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::map<std::string, llvm::Value*> NamedValues;
};

