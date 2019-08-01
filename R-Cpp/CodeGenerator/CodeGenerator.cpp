#include "CodeGenerator.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include <iostream>
#include "../Parser/Parser.h"

using namespace CG;

CodeGenerator::CodeGenerator(Parse::Parser& p):parser_(p), Builder(TheContext), TheModule(std::make_unique<llvm::Module>("RCpp", context())),
                                TheFPM(std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get())),st_(*this)
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

llvm::LLVMContext& CodeGenerator::context()
{
    return TheContext;
}

llvm::IRBuilder<>& CodeGenerator::builder()
{
    return Builder;
}

llvm::Function* CodeGenerator::getFunction(const std::string& Callee)
{
    return TheModule->getFunction(Callee);
}

llvm::Module& CodeGenerator::getModule()
{
    return *TheModule;
}

llvm::legacy::FunctionPassManager* CodeGenerator::FPM()
{
    return TheFPM.get();
}

void CodeGenerator::output() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);
    std::string error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, error);
    if (!Target) {
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
    std::cout << "output to: " << Filename << std::endl;
}

void CodeGenerator::generate()
{
    for(auto& c:parser_.Classes())
    {
        c->generateCode(*this)->print(llvm::errs());
    }
    for(auto& p:parser_.Prototypes())
    {
        p->generateCode(*this)->print(llvm::errs());
    }
    for (auto& c : parser_.Classes()) {
        c->generateFunction_new(*this)->print(llvm::errs());
    }
    for(auto& f:parser_.AST())
    {
        f->generateCode(*this)->print(llvm::errs());
    }
    output();
}
