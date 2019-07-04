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
#include "llvm/Transforms/Utils.h"
class CodeGenerator
{
public:
    CodeGenerator()
        :Builder(TheContext),TheModule(std::make_unique<llvm::Module>("RCpp",context())),
        TheFPM(std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get())) 
    {
        // Promote allocas to registers.
        TheFPM->add(llvm::createPromoteMemoryToRegisterPass());
        // Do simple "peephole" optimizations and bit-twiddling optzns.
        TheFPM->add(llvm::createInstructionCombiningPass());
        // Reassociate expressions.
        TheFPM->add(llvm::createReassociatePass());
        // Eliminate Common SubExpressions.
        TheFPM->add(llvm::createGVNPass());
        // Simplify the control flow graph (deleting unreachable blocks, etc).
        TheFPM->add(llvm::createCFGSimplificationPass());

        TheFPM->doInitialization();
    }

    llvm::LLVMContext& context() {
        return TheContext;
    }

    llvm::AllocaInst* getValue(const std::string& name) {
        return NamedValues[name];
    }

    void setValue(const std::string& name, llvm::AllocaInst* val) {
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

    llvm::legacy::FunctionPassManager* FPM() {
        return TheFPM.get();
    }

private:
    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
};

