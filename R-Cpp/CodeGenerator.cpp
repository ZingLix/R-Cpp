#include "CodeGenerator.h"
#include "Type.h"

llvm::Value* CodeGenerator::binOpGenCode(llvm::Value* LHS, llvm::Value* RHS, OperatorType op) {
    if(is_builtin_type(LHS->getType()->getTypeID())
        &&is_builtin_type(RHS->getType()->getTypeID())) 
    {
        return binOpGenCode_builtin(LHS, RHS, op);
    }
    //todo: Should be operator overload 
    return nullptr;
}

llvm::Value* CodeGenerator::binOpGenCode_builtin(llvm::Value* LHS, llvm::Value* RHS, OperatorType op) {
    llvm::Value* tmp = nullptr;
    if (LHS->getType()->isFloatTy() || LHS->getType()->isDoubleTy()
        || RHS->getType()->isFloatTy() || RHS->getType()->isDoubleTy()) {
        switch (op) {
        case OperatorType::Multiplication:
            return builder().CreateFMul(LHS, RHS);
        case OperatorType::Division:
            return builder().CreateFDiv(LHS, RHS);
        case OperatorType::Addition:
            return builder().CreateFAdd(LHS, RHS);
        case OperatorType::Subtraction:
            return builder().CreateFSub(LHS, RHS);
        case OperatorType::Less:
            return builder().CreateFCmpULT(LHS, RHS);
        case OperatorType::LessEqual:
            return builder().CreateFCmpULE(LHS, RHS);
        case OperatorType::Greater:
            return builder().CreateFCmpUGE(LHS, RHS);
        case OperatorType::GreaterEqual:
            return builder().CreateFCmpUGE(LHS, RHS);
        case OperatorType::Equal:
            return builder().CreateFCmpUEQ(LHS, RHS);
        case OperatorType::NotEqual:
            return builder().CreateFCmpUNE(LHS, RHS);
        case OperatorType::SumComAssign:
            tmp = builder().CreateFAdd(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::MinComAssign:
            tmp = builder().CreateFSub(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::DivComAssign:
            tmp = builder().CreateFDiv(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::RemComAssign:
            tmp = builder().CreateFRem(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::Assignment:
            return builder().CreateStore(RHS, LHS);
        default:
            return nullptr;
        }
    }
    else {
        switch (op) {
        case OperatorType::Multiplication:
            return builder().CreateMul(LHS, RHS);
        case OperatorType::Division:
            return builder().CreateUDiv(LHS, RHS);
        case OperatorType::Remainder:
            return builder().CreateURem(LHS, RHS);
        case OperatorType::Addition:
            return builder().CreateAdd(LHS, RHS);
        case OperatorType::Subtraction:
            return builder().CreateSub(LHS, RHS);
        case OperatorType::LeftShift:
            return builder().CreateShl(LHS, RHS);
        case OperatorType::RightShift:
            return builder().CreateLShr(LHS, RHS);
        case OperatorType::Less:
            return builder().CreateICmpULT(LHS, RHS);
        case OperatorType::LessEqual:
            return builder().CreateICmpULE(LHS, RHS);
        case OperatorType::Greater:
            return builder().CreateICmpUGE(LHS, RHS);
        case OperatorType::GreaterEqual:
            return builder().CreateICmpUGE(LHS, RHS);
        case OperatorType::Equal:
            return builder().CreateICmpEQ(LHS, RHS);
        case OperatorType::NotEqual:
            return builder().CreateICmpNE(LHS, RHS);
        case OperatorType::BitwiseAND:
        case OperatorType::LogicalAND:
            return builder().CreateAnd(LHS, RHS);
        case OperatorType::BitwiseOR:
        case OperatorType::LogicalOR:
            return builder().CreateOr(LHS, RHS);
        case OperatorType::SumComAssign:
            tmp = builder().CreateAdd(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::MinComAssign:
            tmp = builder().CreateSub(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::DivComAssign:
            tmp = builder().CreateUDiv(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::RemComAssign:
            tmp = builder().CreateURem(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::LshComAssign:
            tmp = builder().CreateShl(LHS, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::RshComAssign:
            tmp = builder().CreateLShr(tmp, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::ANDComAssign:
            tmp = builder().CreateAnd(tmp, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::XORComAssign:
            tmp = builder().CreateXor(tmp, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::ORComAssign:
            tmp = builder().CreateOr(tmp, RHS);
            return builder().CreateStore(tmp, LHS);
        case OperatorType::Assignment:
            return builder().CreateStore(RHS, LHS);
        default:
            return nullptr;
        }
    }
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
