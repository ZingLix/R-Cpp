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
#include <llvm/Support/TargetSelect.h>
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include <iostream>

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

    void output() {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        TheModule->setTargetTriple(TargetTriple);
        std::string error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple,error);
        if(!Target) {
            llvm::errs() << error;
            return;
        }
        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions opt;
        auto RM = llvm::Optional<llvm::Reloc::Model>();
        auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
        TheModule->setDataLayout(TheTargetMachine->createDataLayout());
        auto Filename = "output.o";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::F_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*TheModule);
        dest.flush();
        std::cout << "output to: "<< Filename << std::endl;
    }

private:
    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
};

