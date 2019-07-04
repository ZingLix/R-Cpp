#include "AST.h"
#include "CodeGenerator.h"
#include "Type.h"
#include "llvm/IR/Constants.h"
#include <iostream>
using namespace llvm;

Value* LogError(const std::string& msg) {
    std::cout << msg<<std::endl;
    return nullptr;
}

Function* LogErrorF(const std::string& msg) {
    std::cout << msg << std::endl;
    return nullptr;
}

AllocaInst* CreateEntryBlockAlloca(Function* TheFunction, Type* Type,
    const std::string& VarName, CodeGenerator& cg) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
        TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type, 0,
        VarName.c_str());
}

AllocaInst* CreateEntryBlockAlloca(Function* TheFunction,const std::string& TypeName,
    const std::string& VarName,CodeGenerator& cg) {
    return CreateEntryBlockAlloca(TheFunction, get_builtin_type(TypeName,cg), VarName, cg);
}

IntegerExprAST::IntegerExprAST(int v): val(v) {
}

Value* IntegerExprAST::generateCode(CodeGenerator& cg) {
    return ConstantInt::get(cg.context(), APInt(32,val,true));
}

FloatExprAST::FloatExprAST(float v): val(v) {
}

Value* FloatExprAST::generateCode(CodeGenerator& cg) {
    return ConstantFP::get(cg.context(), APFloat(val));
}

VaribleExprAST::VaribleExprAST(const std::string& n): name(n) {
}

llvm::Value* VaribleExprAST::generateCode(CodeGenerator& cg) {
    auto v = cg.getValue(name);
    if (v == nullptr) std::cout << "Unknown variable name." << std::endl;
    return cg.builder().CreateLoad(v,name.c_str());
}

VaribleDefAST::VaribleDefAST(const std::string& type_name, const std::string& var_name,
                             std::unique_ptr<ExprAST> init_value)
    : typename_(type_name), varname_(var_name),init_value_(std::move(init_value)) 
{
}

VaribleDefAST::VaribleDefAST(const std::string& type_name, const std::string& var_name)
    :typename_(type_name),varname_(var_name),init_value_(std::move(nullptr)) 
{
}

llvm::Value* VaribleDefAST::generateCode(CodeGenerator& cg) {
    auto F = cg.builder().GetInsertBlock()->getParent();
    Value* InitVal;
    if(!init_value_) {
        InitVal = get_builtin_type_default_value(typename_, cg);
    }else {
        InitVal = init_value_->generateCode(cg);
        if (!InitVal) return nullptr;
    }
    auto alloca = CreateEntryBlockAlloca(F, typename_, varname_, cg);
    cg.builder().CreateStore(InitVal, alloca);
    cg.setValue(varname_, alloca);
}

BinaryExprAST::BinaryExprAST(TokenType op, std::unique_ptr<ExprAST> lhs, 
                             std::unique_ptr<ExprAST> rhs)
    : Op(op), LHS(std::move(lhs)),RHS(std::move(rhs)) 
{
}

llvm::Value* BinaryExprAST::generateCode(CodeGenerator& cg) {
    if(Op==TokenType::Equal) {
        VaribleExprAST* LHSE = dynamic_cast<VaribleExprAST*>(LHS.get());
        if (!LHSE)
            return LogError("destination of '=' must be a variable");
        auto val = RHS->generateCode(cg);
        if (!val) return nullptr;
        auto var = cg.getValue(LHSE->getName());
        if (!var) return LogError("Unknown variable name.");
        cg.builder().CreateStore(val, var);
        return val;
    }
    auto L = LHS->generateCode(cg);
    auto R = RHS->generateCode(cg);
    if (!L || !R) return nullptr;
    auto& builder = cg.builder();
    bool isFloat = L->getType()->isFloatTy() || R->getType()->isFloatTy();
    if(isFloat) {
        switch (Op) {
        case TokenType::Plus:
            return builder.CreateFAdd(L, R);
        case TokenType::Minus:
            return builder.CreateFSub(L, R);
        case TokenType::Multiply:
            return builder.CreateFMul(L, R);
        case TokenType::lAngle:
            return builder.CreateFCmpULT(L, R);
        default:
            return LogError("Invalid binary operator");
        }
    }else {
        switch (Op) {
        case TokenType::Plus:
            return builder.CreateAdd(L, R);
        case TokenType::Minus:
            return builder.CreateSub(L, R);
        case TokenType::Multiply:
            return builder.CreateMul(L, R);
        case TokenType::lAngle:
            return builder.CreateICmpULT(L, R);
        default:
            return LogError("Invalid binary operator");
        }
    }

}

ReturnAST::ReturnAST(std::unique_ptr<ExprAST> returnValue): ret_val_(std::move(returnValue)) 
{
}

Value* ReturnAST::generateCode(CodeGenerator& cg) {
    auto retval = ret_val_->generateCode(cg);
    if(retval!=nullptr)
        return cg.builder().CreateRet(retval);
    return nullptr;
}

CallExprAST::
CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
    : Callee(callee),Args(std::move(args)) 
{
}

llvm::Value* BlockExprAST::generateCode(CodeGenerator& cg) {
    Value* ret = nullptr;
    for(auto&s:expr_) {
        auto v= s->generateCode(cg);
        if (ret == nullptr) ret = v;
        if (!v)
            return nullptr;
    }
    return ret;
}

llvm::Value* IfExprAST::generateCode(CodeGenerator& cg) {
    auto cond = Cond->generateCode(cg);
    if (!cond) return nullptr;
    auto& builder = cg.builder();
    cond = builder.CreateICmpEQ(cond,ConstantInt::get(cg.context(),APInt(32,0)), "ifcond");
    Function* F = builder.GetInsertBlock()->getParent();
    BasicBlock* ThenBB = BasicBlock::Create(cg.context(), "then", F);
    BasicBlock* ElseBB = BasicBlock::Create(cg.context(), "else");
    BasicBlock* MergeBB = BasicBlock::Create(cg.context(), "ifcont");
    builder.CreateCondBr(cond, ThenBB,  Else==nullptr? MergeBB:ElseBB);
    builder.SetInsertPoint(ThenBB);
    auto thenV= Then->generateCode(cg);
    builder.CreateBr(MergeBB);
    ThenBB = builder.GetInsertBlock();
    Value* elseV = nullptr;
    if(Else!=nullptr) {
        F->getBasicBlockList().push_back(ElseBB);
        builder.SetInsertPoint(ElseBB);
        if (Else != nullptr) {
            elseV = Else->generateCode(cg);
            if (!elseV) return nullptr;
        }
        builder.CreateBr(MergeBB);
        ElseBB = builder.GetInsertBlock();
    }
    F->getBasicBlockList().push_back(MergeBB);
    builder.SetInsertPoint(MergeBB);
    PHINode* PN = builder.CreatePHI(get_builtin_type("i32",cg), 2, "ifcond");
    PN->addIncoming(thenV, ThenBB);
    if(Else!=nullptr)
        PN->addIncoming(elseV, ElseBB);
    return PN;
    
}


llvm::Value* CallExprAST::generateCode(CodeGenerator& cg) {
    auto CalleeF = cg.getFunction(Callee);
    if (!CalleeF) return LogError("Unknown function referenced.");

    if (CalleeF->arg_size() != Args.size())
        return LogError("Arguments count mismatch.");

    std::vector<Value*> Argv;
    for(unsigned i=0;i<Args.size();++i) {
        Argv.push_back(Args[i]->generateCode(cg));
        if(!Argv.back()) return nullptr;
    }
    return cg.builder().CreateCall(CalleeF, Argv, "calltmp");
}

llvm::Function* PrototypeAST::generateCode(CodeGenerator& cg) {
    std::vector<llvm::Type*> ArgT;
    for(auto&t:Args) {
        if (t.first == "i32") ArgT.push_back(Type::getInt32Ty(cg.context()));
        else if (t.first == "float") ArgT.push_back(Type::getFloatTy(cg.context()));
        else return LogErrorF("Unknown type.");
    }
    auto FT = FunctionType::get(Type::getDoubleTy(cg.context()), ArgT, false);
    auto F = Function::Create(FT, Function::ExternalLinkage, Name, cg.getModule());
    int i = 0;
    for (auto& arg : F->args())
        arg.setName(Args[i++].second);
    return F;
}

llvm::Function* FunctionAST::generateCode(CodeGenerator& cg) {
    auto F = cg.getFunction(Proto->getName());
    if (!F) F = Proto->generateCode(cg);
    if (!F) return nullptr;
    if (!F->empty()) return LogErrorF("Cannot redefine function.");
    BasicBlock* BB = BasicBlock::Create(cg.context(), "entry", F);
    cg.builder().SetInsertPoint(BB);
    cg.clearValue();
    for(auto& Arg:F->args()) {
        auto alloca = CreateEntryBlockAlloca(F,Arg.getType(), Arg.getName(), cg);
        cg.builder().CreateStore(&Arg, alloca);
        cg.setValue(Arg.getName(), alloca);
    }
    Body->generateCode(cg);
    verifyFunction(*F);
    cg.FPM()->run(*F);
    return F;
}
