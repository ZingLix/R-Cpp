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

CodeGenerator::CodeGenerator(Parse::Parser& p):context_(p.context()), Builder(TheContext), TheModule(std::make_unique<llvm::Module>("RCpp", context())),
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

llvm::Type* CodeGenerator::getBuiltinType(const std::string& s)
{
    if (s == "i32") return llvm::Type::getInt32Ty(this->context());
    if (s == "i64") return llvm::Type::getInt64Ty(this->context());
    if (s == "u32") return llvm::Type::getInt32Ty(this->context());
    if (s == "u64") return llvm::Type::getInt64Ty(this->context());
    if (s == "bool") return llvm::Type::getInt1Ty(this->context());
    if (s == "float") return llvm::Type::getFloatTy(this->context());
    if (s == "double") return llvm::Type::getDoubleTy(this->context());
    if (s == "void") return llvm::Type::getVoidTy(this->context());
    return nullptr;
}

llvm::Value* CodeGenerator::getBuiltinTypeDefaultValue(const std::string& s)
{
    if (s == "i32") return llvm::ConstantInt::get(this->context(), llvm::APInt(32, 0, true));
    if (s == "i64") return llvm::ConstantInt::get(this->context(), llvm::APInt(64, 0, true));
    if (s == "u32") return llvm::ConstantInt::get(this->context(), llvm::APInt(32, 0, false));
    if (s == "u64") return llvm::ConstantInt::get(this->context(), llvm::APInt(32, 0, false));
    if (s == "bool") return llvm::ConstantInt::get(this->context(), llvm::APInt(1, 0, false));
    if (s == "float") return llvm::ConstantFP::get(this->context(), llvm::APFloat(0.0));
    if (s == "double") return llvm::ConstantFP::get(this->context(), llvm::APFloat(0.0));
    return nullptr;
}

void CodeGenerator::generate()
{
    for(auto& c: *context_.Class())
    {
        c->generateCode(*this)->print(llvm::errs());
        std::cout << std::endl;
    }
    for(auto& p: *context_.Prototype())
    {
        p->generateCode(*this)->print(llvm::errs());
    }
    for (auto& c : *context_.Class()) {
        c->generateFunction_new(*this)->print(llvm::errs());
        std::cout << std::endl;
    }
    //for (auto& c : context_.Classes()) {
    //    c->generateFunction_new(*this)->print(llvm::errs());
    //}
    for(auto& f: *context_.Function())
    {
        f->generateCode(*this)->print(llvm::errs());
    }
    output();
}
