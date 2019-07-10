#include "CodeGenerator.h"
#include "Type.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"

llvm::Value* CodeGenerator::binOpGenCode(llvm::Value* LHS, llvm::Value* RHS, OperatorType op, const std::string& LHSName) {
    if(is_builtin_type(LHS->getType()->getTypeID())
        &&is_builtin_type(RHS->getType()->getTypeID())) 
    {
        return binOpGenCode_builtin(LHS, RHS, op,LHSName);
    }
    //todo: Should be operator overload 
    return nullptr;
}

llvm::Value* CodeGenerator::binOpGenCode_builtin(llvm::Value* LHS, llvm::Value* RHS, OperatorType op, const std::string& LHSName) {
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
            return assign(LHSName,tmp);
        case OperatorType::MinComAssign:
            tmp = builder().CreateFSub(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::DivComAssign:
            tmp = builder().CreateFDiv(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::RemComAssign:
            tmp = builder().CreateFRem(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::Assignment:
            return builder().CreateStore(RHS, LHS);
        default:
            return nullptr;
        }
    }
    else {
        if(llvm::isa<llvm::ConstantInt>(LHS)) {
            LHS = llvm::ConstantInt::get(LHS->getType(), 
                llvm::APInt(RHS->getType()->getIntegerBitWidth(),
                    static_cast<llvm::ConstantInt*>(LHS)->getSExtValue(), true));
        }else
        if (llvm::isa<llvm::ConstantInt>(RHS)) {
            RHS = llvm::ConstantInt::get(RHS->getType(), 
                llvm::APInt(LHS->getType()->getIntegerBitWidth(),
                    static_cast<llvm::ConstantInt*>(RHS)->getSExtValue(), true));
        }
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
            return assign(LHSName,tmp);
        case OperatorType::MinComAssign:
            tmp = builder().CreateSub(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::DivComAssign:
            tmp = builder().CreateUDiv(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::RemComAssign:
            tmp = builder().CreateURem(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::LshComAssign:
            tmp = builder().CreateShl(LHS, RHS);
            return assign(LHSName,tmp);
        case OperatorType::RshComAssign:
            tmp = builder().CreateLShr(tmp, RHS);
            return assign(LHSName,tmp);
        case OperatorType::ANDComAssign:
            tmp = builder().CreateAnd(tmp, RHS);
            return assign(LHSName,tmp);
        case OperatorType::XORComAssign:
            tmp = builder().CreateXor(tmp, RHS);
            return assign(LHSName,tmp);
        case OperatorType::ORComAssign:
            tmp = builder().CreateOr(tmp, RHS);
            return assign(LHSName,tmp);
        case OperatorType::Assignment:
            return assign(LHSName, RHS);;

        default:
            return nullptr;
        }
    }
}

llvm::Value* CodeGenerator::assign(const std::string& varname, llvm::Value* val) {
    auto var = getValue(varname).alloc;
    if (llvm::isa<llvm::ConstantInt>(val)) {
        val = llvm::ConstantInt::get(val->getType(),
            llvm::APInt(var->getAllocatedType()->getIntegerBitWidth(),
                static_cast<llvm::ConstantInt*>(val)->getSExtValue(), true));
    }
    if (!var) return nullptr;
    builder().CreateStore(val, var);
    return val;
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
