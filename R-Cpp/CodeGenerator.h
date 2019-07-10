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
#include <iostream>

class CodeGenerator
{
public:
    CodeGenerator()
        :Builder(TheContext),TheModule(std::make_unique<llvm::Module>("RCpp",context())),
        TheFPM(std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get())) 
    {
        NamedValues.emplace_back();
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

    Variable getValue(const std::string& name) {
        for(auto it=NamedValues.rbegin();it!=NamedValues.rend();++it)
        {
            auto t = it->find(name);
            if (t != it->end()) return t->second;
        }
        return Variable();
    }

    void setValue(const std::string& name, Variable val) {
        NamedValues.back()[name] = val;
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

    llvm::Value* binOpGenCode(llvm::Value* LHS, llvm::Value* RHS, OperatorType op);

    void output();

    void createNewScope()
    {
        NamedValues.emplace_back();
    }

    void destoryScope()
    {
        NamedValues.pop_back();
    }

private:
    llvm::Value* binOpGenCode_builtin(llvm::Value* LHS, llvm::Value* RHS, OperatorType op);


    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    std::vector<std::map<std::string, Variable>> NamedValues;
//    std::map<std::string, llvm::AllocaInst*> NamedValues;
};

class ScopeGuard
{
public:
    ScopeGuard(CodeGenerator& cg)
        :cg_(cg)
    {
        cg_.createNewScope();
    }

    ~ScopeGuard()
    {
        cg_.destoryScope();
    }

private:
    CodeGenerator& cg_;
};