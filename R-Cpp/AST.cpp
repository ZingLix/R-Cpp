#include "AST.h"
#include "CodeGenerator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <iostream>
using namespace llvm;

Value* LogError(const std::string& msg) {
    std::cout << msg<<std::endl;
    return nullptr;
}

llvm::Function* LogErrorF(const std::string& msg) {
    std::cout << msg << std::endl;
    return nullptr;
}

AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction, Type* Type,
    const std::string& VarName, CodeGenerator& cg) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
        TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type, 0,
        VarName.c_str());
}

AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction,const std::string& TypeName,
    const std::string& VarName,CodeGenerator& cg) {
    auto type = get_type(TypeName,cg);
    return CreateEntryBlockAlloca(TheFunction, type, VarName, cg);
}

IntegerExprAST::IntegerExprAST(std::int64_t v):ExprAST(VarType("i32")), val(v) {
}

Value* IntegerExprAST::generateCode(CodeGenerator& cg) {
    return ConstantInt::get(cg.context(), APInt(32,val,true));
}

FloatExprAST::FloatExprAST(double v):ExprAST(VarType("double")), val(v) {
}

Value* FloatExprAST::generateCode(CodeGenerator& cg) {
    return ConstantFP::get(cg.context(), APFloat(val));
}

VariableExprAST::VariableExprAST(const std::string& n,const VarType& t)
    :ExprAST(t), name(n) {
}

llvm::Value* VariableExprAST::generateCode(CodeGenerator& cg) {
    auto v = cg.symbol().getValue(name);
    if (v.alloc == nullptr) {
        std::cout << "Unknown variable name: "<<name<<"." << std::endl;
        return nullptr;
    }
    alloca_ = v.alloc;
    return cg.builder().CreateLoad(v.alloc);

}

VariableDefAST::VariableDefAST(const VarType& type, const std::string& var_name,
                             std::unique_ptr<ExprAST> init_value)
    :ExprAST(VarType("void")), type_(type), varname_(var_name),init_value_(std::move(init_value)) 
{
}

VariableDefAST::VariableDefAST(const VarType& type, const std::string& var_name)
    :ExprAST(VarType("void")), type_(type),varname_(var_name),init_value_(std::move(nullptr)) 
{
}

llvm::Value* VariableDefAST::generateCode(CodeGenerator& cg) {
    if(type_.typeName=="Arr")
    {
        auto elementType = get_type(type_.templateArgs[0],cg);
        auto size = std::stoi(type_.templateArgs[1].typeName);
        //auto size = template_args_[1]->generateCode(cg);
        auto type = ArrayType::get(elementType, size);
        auto alloc = cg.builder().CreateAlloca(type);
        cg.symbol().setValue(varname_, Variable(varname_, type_, alloc));
    }else if(type_.typeName=="__ptr")
    {
        auto elementType = get_type(type_.templateArgs[0], cg);
        auto type = PointerType::get(elementType, 0);
        auto alloc = cg.builder().CreateAlloca(type);
        cg.symbol().setValue(varname_, Variable(varname_, type_, alloc));
        if(init_value_)
        {
            auto InitVal = init_value_->generateCode(cg);
            if (!InitVal) return nullptr;
            cg.builder().CreateStore(InitVal, alloc);
        }
    }
    else
    {
        auto type = get_type(type_,cg);
        auto alloc = cg.builder().CreateAlloca(type, nullptr, varname_);
       // auto F = cg.builder().GetInsertBlock()->getParent();
       // auto alloc = CreateEntryBlockAlloca(F, typename_, varname_, cg);
        if (init_value_) {
            auto InitVal = init_value_->generateCode(cg);
            if (!InitVal) return nullptr;
            cg.builder().CreateStore(InitVal, alloc);
        }
        cg.symbol().setValue(varname_, Variable(varname_, type_, alloc));
    }

    return Constant::getNullValue(Type::getDoubleTy(cg.context()));
}

BinaryExprAST::BinaryExprAST(OperatorType op,
    std::unique_ptr<ExprAST> lhs,
    std::unique_ptr<ExprAST> rhs, const std::string& retType)
    :ExprAST(retType), Op(op), LHS(std::move(lhs)),RHS(std::move(rhs))
{
}

llvm::Value* BinaryExprAST::generateCode(CodeGenerator& cg) {
    auto L = LHS->generateCode(cg);
    auto R = RHS->generateCode(cg);
    auto Ltype = LHS->getTypeName();
    auto Rtype = RHS->getTypeName();
    if (!L || !R) return nullptr;
    Value* res=nullptr;
    if (isCompoundAssignOperator(Op)) {
        auto LHSE = dynamic_cast<AllocAST*>(LHS.get());
        if (!LHSE)
           return LogError("destination of '=' must be a variable");
        auto op = compoundAssignToOperator(Op);
        res = builtinTypeOperate(L, Ltype, R, Rtype, op, cg);
        //auto var = cg.symbol().getValue(LHSE->getName());
        //if (!var.alloc) return LogError("Unknown variable name.");
        return cg.builder().CreateStore(res, LHSE->getAlloc());;

    } else if(Op==OperatorType::Assignment)
    {
        auto LHSE = dynamic_cast<AllocAST*>(LHS.get());
        if (!LHSE)
            return LogError("destination of '=' must be a variable");
        //auto var = cg.symbol().getValue();
        //if (!var.alloc) return LogError("Unknown variable name.");
        return cg.builder().CreateStore(R, LHSE->getAlloc());;
    }
    else{
        res = builtinTypeOperate(L, Rtype, R, Rtype, Op,cg);
    }
    if(!res) {
        return LogError("No suitable operator.");
    }
    return res;

}

ReturnAST::ReturnAST(std::unique_ptr<ExprAST> returnValue):ExprAST(VarType("null")), ret_val_(std::move(returnValue)) 
{
}

Value* ReturnAST::generateCode(CodeGenerator& cg) {
    auto retval = ret_val_->generateCode(cg);
    if(retval!=nullptr)
        return cg.builder().CreateRet(retval);
    return nullptr;
}

llvm::Value* ForExprAST::generateCode(CodeGenerator& cg) {
    SymbolTable::ScopeGuard sg(cg.symbol());
    auto startval = Start->generateCode(cg);
    if (!startval) return nullptr;
    auto F = cg.builder().GetInsertBlock()->getParent();
    auto PreHeaderBB = BasicBlock::Create(cg.context(), "cond", F);
    cg.builder().CreateBr(PreHeaderBB);
    cg.builder().SetInsertPoint(PreHeaderBB);
    auto EndCond = Cond->generateCode(cg);
    EndCond = cg.builder().CreateICmpNE(EndCond,
        ConstantInt::get(get_builtin_type("bool", cg), APInt(1, 0)), "loopcond");
    auto LoopBB = BasicBlock::Create(cg.context(), "loop", F);
    auto AfterBB = BasicBlock::Create(cg.context(), "afterloop", F);
    cg.builder().CreateCondBr(EndCond, LoopBB, AfterBB);
    cg.builder().SetInsertPoint(LoopBB);
    Body->generateCode(cg);
    End->generateCode(cg);
    cg.builder().CreateBr(PreHeaderBB);
    cg.builder().SetInsertPoint(AfterBB);
    return Constant::getNullValue(Type::getDoubleTy(cg.context()));
}

CallExprAST::
CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args, const VarType& t)
    :ExprAST(t), Callee(callee),Args(std::move(args)),thisPtr(nullptr)
{
}

void CallExprAST::setThis(std::unique_ptr<ExprAST> This)
{
    thisPtr = std::move(This);
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

std::vector<std::unique_ptr<ExprAST>>& BlockExprAST::instructions()
{
    return expr_;
}

llvm::Value* IfExprAST::generateCode(CodeGenerator& cg) {
    SymbolTable::ScopeGuard sg(cg.symbol());
    auto cond = Cond->generateCode(cg);
    if (!cond) return nullptr;
    auto& builder = cg.builder();
    if(!llvm::isa<IntegerType>(*cond->getType())) {
        return LogError("Only bool or integer can be the condition.");
    }
    cond = builder.CreateICmpNE(cond,ConstantInt::get(cg.context(),
        APInt(static_cast<IntegerType*>(cond->getType())->getBitWidth(),0)), "ifcond");
    llvm::Function* F = builder.GetInsertBlock()->getParent();
    BasicBlock* ThenBB = BasicBlock::Create(cg.context(), "then", F);
    BasicBlock* ElseBB = BasicBlock::Create(cg.context(), "else");
    BasicBlock* MergeBB = BasicBlock::Create(cg.context(), "ifcont");
    builder.CreateCondBr(cond, ThenBB,  Else==nullptr? MergeBB:ElseBB);
    auto genCode = [&](std::unique_ptr<BlockExprAST>& block) {
        bool anotherTerminator = false;
        for (auto& expr : block->instructions()) {
            auto v = expr->generateCode(cg);
            if (isa<ReturnInst>(v)) {
                anotherTerminator = true;
                break;
            }
        }
        if (!anotherTerminator)
            builder.CreateBr(MergeBB);
    };
    builder.SetInsertPoint(ThenBB);
    genCode(Then);
    ThenBB = builder.GetInsertBlock();
    if(Else!=nullptr) {
        F->getBasicBlockList().push_back(ElseBB);
        builder.SetInsertPoint(ElseBB);
        genCode(Else);
        ElseBB = builder.GetInsertBlock();
    }
    F->getBasicBlockList().push_back(MergeBB);
    builder.SetInsertPoint(MergeBB);
    //PHINode* PN = builder.CreatePHI(get_builtin_type("i32",cg), 2, "ifcond");
   // PN->addIncoming(thenV, ThenBB);
    //if(Else!=nullptr)
    //    PN->addIncoming(elseV, ElseBB);
    return F;
    
}


llvm::Value* CallExprAST::generateCode(CodeGenerator& cg) {
    auto CalleeF = cg.getFunction(Callee);
    if (!CalleeF) return LogError("Unknown function referenced.");
    std::vector<Value*> Argv;
    if (thisPtr)
    {
        thisPtr->generateCode(cg);
        Argv.push_back(dynamic_cast<AllocAST*>(thisPtr.get())->getAlloc());
    }
    for(unsigned i=0;i<Args.size();++i) {
        Argv.push_back(Args[i]->generateCode(cg));
        if(!Argv.back()) return nullptr;
    }
    return cg.builder().CreateCall(CalleeF, Argv);
}

llvm::Function* PrototypeAST::generateCode(CodeGenerator& cg) {
    F.name = ::Function::mangle(F);
    auto p = cg.getFunction(F.name);
    if (p) return p;
    std::vector<llvm::Type*> ArgT;
    if (F.classType.typeName != "") 
        ArgT.push_back(PointerType::getUnqual(cg.symbol().getLLVMType(F.classType)));
    for(auto& t:F.args) {
        auto type = get_type(t.type,cg);
       // if (!type) type = get_builtin_type(t.type,cg);
        if(!type) return LogErrorF("Unknown type.");
        ArgT.push_back(type);
    }
    auto FT = FunctionType::get(get_type(F.returnType,cg), ArgT, false);
    auto Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, F.name , cg.getModule());
    int i = F.classType.typeName == "" ?0:-1;
    for (auto& arg : Func->args())
    {
        if (i == -1) arg.setName("this");
        else arg.setName(F.args[i].name);
        ++i;
    }
    if(F.classType.typeName!="")
    {
        F.args.insert(F.args.begin(), Variable("this", F.classType));
    }
    F.alloc = Func;
    cg.symbol().addFunction(F.name, F);
    return Func;
}

llvm::Function* FunctionAST::generateCode(CodeGenerator& cg) {
    auto F = cg.getFunction(functionName);
    auto Func = cg.symbol().getMangledFunction(functionName);
    if (!F) LogError("No function named " + functionName+".");
    //if (!F) F = Proto->generateCode(cg);
    //if (!F) return nullptr;
    //if (!F->empty()) return LogErrorF("Cannot redefine function.");
    if (!Body) return F;
    BasicBlock* BB = BasicBlock::Create(cg.context(), "entry", F);
    SymbolTable::ScopeGuard sg(cg.symbol());
    cg.builder().SetInsertPoint(BB);
    int i = 0;
    for(auto& Arg:F->args()) {
        auto alloca = CreateEntryBlockAlloca(F,Arg.getType(), Arg.getName(), cg);
        cg.builder().CreateStore(&Arg, alloca);
        cg.symbol().setValue(Func.args[i].name, 
            Variable(Func.args[i].name, Func.args[i].type,alloca));
        i++;
    }
    bool hasReturn = false;
    if(Func.classType.typeName!="")
    {
        auto c = cg.symbol().getClass(Func.classType);
        int i = 0;
        for(auto& v:c.memberVariables)
        {
            Value* index = ConstantInt::get(cg.context(),
                APInt(32, i++));
            Value* data = cg.symbol().getValue("this").alloc;
            data = cg.builder().CreateLoad(data);
            std::vector<llvm::Value*> indices(2);
            indices[0] = llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, true));
            indices[1] = index;
            Value* ptr = cg.builder().CreateGEP(data, indices, "memberptr");
            cg.symbol().setValue(v.name, Variable(v.name, v.type, static_cast<AllocaInst*>(ptr)));
        }
        
    }
    for(auto& i:Body->instructions()) {
        i->generateCode(cg);
        if(dynamic_cast<ReturnAST*>(i.get())!=nullptr) {
            hasReturn = true;
        }
    }
    // fn func(){
    //     if(..) return;
    //     else return;
    //     // here doesn't need return, but ifexpr will create new branch 
    //     // for the code below. LLVM requires that all blocks mush have
    //     // terminator. Therefore, using 'return void' to make it valid,
    //     // and optimizer will remove it.
    //     // This is a temporary method. Maybe ifexpr.generateCode() should
    //     // be refactored.
    // }
    //
    if (!hasReturn) cg.builder().CreateRet(nullptr);
    if(llvm::verifyFunction(*F,&errs())) {
        std::cout<<std::endl << "something bad happened ...\n";
    }
    cg.FPM()->run(*F);
    return F;
}

llvm::StructType* ClassAST::generateCode(CodeGenerator& cg)
{
    auto type = StructType::create(cg.context(), c.type.typeName);
    cg.symbol().addClass(c.type, c);
    std::vector<Type*> members;
    for(auto m:c.memberVariables)
    {
        members.push_back(get_type(m.type, cg));
        //auto t = get_builtin_type(m.type, cg);
        //if (!t)
        //    members.push_back(cg.symbol().getType(m.type));
        //else members.push_back(t);
    }
    type->setBody(members);
    cg.symbol().setLLVMType(c.type,type);
    //for(auto fn:c.memberFunctions)
    //{
    //    cg.symbol().getFunction(fn.name)
    //}
   // generateFunction_new(cg);
    return type;
}

llvm::Value* ClassAST::generateFunction_new(CodeGenerator& cg)
{
    ::Function f;
    f.name = "new";
    f.classType = c.type;
    VarType retType("__ptr");
    retType.templateArgs.push_back(VarType(c.type.typeName));
    f.name = ::Function::mangle(f);
    auto FT = FunctionType::get(get_type(retType, cg), std::vector<Type*>(), false);
    auto Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, f.name, cg.getModule());
    f.alloc = Func;
    cg.symbol().addFunction(::Function::mangle(f), f);
    BasicBlock* BB = BasicBlock::Create(cg.context(), "entry", Func);
    cg.builder().SetInsertPoint(BB);
    std::vector<Value*> index;
    index.push_back(ConstantInt::get(cg.context(), APInt(32, 1)));
    auto size = cg.builder().CreateGEP(Constant::getNullValue(PointerType::get(get_type(c.type, cg), 0)), index);
    size = cg.builder().CreateCast(llvm::Instruction::CastOps::PtrToInt, size, get_builtin_type("i32", cg));
    std::vector<Value*> args;
    args.push_back(size);
    auto retVal = cg.builder().CreateCall(cg.getFunction("_R6mallocI3i32"), args);
    cg.builder().CreateRet(retVal);
    cg.FPM()->run(*Func);
    return Func;
}


MemberAccessAST::MemberAccessAST(std::unique_ptr<ExprAST> Var, std::string Member, OperatorType Op,
                                 const VarType& retType): ExprAST(retType), var(std::move(Var)),
                                                              member(std::move(Member)), op(Op)
{
}

llvm::Value* MemberAccessAST::generateCode(CodeGenerator& cg)
{
    var->generateCode(cg);
    alloca_ = dynamic_cast<AllocAST*>(var.get())->getAlloc();
    Value* index = ConstantInt::get(cg.context(),
        APInt(32, cg.symbol().getClassMemberIndex(var->getType(), member)));
    Value* data = var->generateCode(cg);
    std::vector<llvm::Value*> indices(2);
    indices[0] = llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, true));
    indices[1] = index;
    Value* ptr = cg.builder().CreateGEP(alloca_, indices, "memberptr");
    alloca_ = static_cast<AllocaInst*>(ptr);
    return cg.builder().CreateLoad(ptr);
}

llvm::Value* UnaryExprAST::generateCode(CodeGenerator& cg)
{
    auto var = expr->generateCode(cg);
    if(op==OperatorType::Subscript)
    {
        std::vector<Value*> values;
        values.push_back(ConstantInt::get(cg.context(), llvm::APInt(32, 0, true)));
        for (auto& a : args) {
            values.push_back(a->generateCode(cg));
        }
        Value* ptr = cg.builder().CreateGEP(dynamic_cast<VariableExprAST*>(expr.get())->getAlloc(), values);
        alloca_ = static_cast<AllocaInst*>(ptr);
        type = expr->getType().templateArgs[0];
        return cg.builder().CreateLoad(ptr);
    }else if(op==OperatorType::PreIncrement)
    {
        auto alloc = dynamic_cast<AllocAST*>(expr.get())->getAlloc();
        auto res = cg.builder().CreateAdd(var, ConstantInt::get(cg.context(), APInt(32, 1, true)));
        cg.builder().CreateStore(res, alloc);
        return cg.builder().CreateLoad(alloc);
    }else if(op==OperatorType::PreDecrement)
    {
        auto alloc = dynamic_cast<AllocAST*>(expr.get())->getAlloc();
        auto res = cg.builder().CreateSub(var, ConstantInt::get(cg.context(), APInt(32, 1, true)));
        cg.builder().CreateStore(res, alloc);
        return cg.builder().CreateLoad(alloc);
    }else if(op==OperatorType::PostIncrement)
    {
        auto alloc = dynamic_cast<AllocAST*>(expr.get())->getAlloc();
        auto tmp = cg.builder().CreateLoad(alloc);
        auto res= cg.builder().CreateAdd(var, ConstantInt::get(cg.context(), APInt(32, 1, true)));
        cg.builder().CreateStore(res, alloc);
        return tmp;
    }else if(op==OperatorType::PostDecrement)
    {
        auto alloc = dynamic_cast<AllocAST*>(expr.get())->getAlloc();
        auto tmp = cg.builder().CreateLoad(alloc);
        auto res = cg.builder().CreateSub(var, ConstantInt::get(cg.context(), APInt(32, 1, true)));
        cg.builder().CreateStore(res, alloc);
        return tmp;
    }else if(op==OperatorType::Promotion)
    {
        return var;
    }else if(op==OperatorType::Negation)
    {
        return cg.builder().CreateSub(ConstantInt::get(cg.context(), APInt(0, 32, true)), var);
    }else if(op==OperatorType::LogicalNOT)
    {
        return cg.builder().CreateNot(var);
    }else if(op==OperatorType::BitwiseNOT)
    {
        return cg.builder().CreateNot(var);
    }else if(op==OperatorType::Dereference)
    {
        alloca_ = static_cast<AllocaInst*>(var);
        return cg.builder().CreateLoad(var);
    }
    LogError("Unknown unary operator.");
    return nullptr;
}

llvm::Value* NamespaceExprAST::generateCode(CodeGenerator& cg) {
    LogError("Namespace cannot generate code.");
    return nullptr;
}
