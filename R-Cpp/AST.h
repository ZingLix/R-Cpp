#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Token.h"
#include "Operator.h"
#include <llvm/IR/Value.h>
#include "CodeGenerator.h"

class ExprAST
{
public:
    ExprAST(const std::string& s):type(s){}
    std::string getType()
    {
        return type;
    }
    virtual ~ExprAST() = default;
    virtual llvm::Value* generateCode(CodeGenerator& cg) =0;
protected:
    std::string type;
};

class AllocAST
{
public:
    AllocAST():alloca_(nullptr){}
    llvm::AllocaInst* getAlloc() { return alloca_; }
protected:
    llvm::AllocaInst* alloca_;
};

class IntegerExprAST:public ExprAST
{
public:
    IntegerExprAST(std::int64_t v);
    llvm::Value* generateCode(CodeGenerator& cg) override;
    std::int64_t value() { return val; }
private:
    std::int64_t val;
};

class FloatExprAST :public ExprAST
{
public:
    FloatExprAST(double v);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    double val;
};

class VariableExprAST:public ExprAST,public AllocAST
{
public:
    VariableExprAST(const std::string& n, const std::string& t);
    llvm::Value* generateCode(CodeGenerator& cg) override;
    std::string getName() { return name; }

private:
    std::string name;
};

class BlockExprAST
{
public:
    BlockExprAST(std::vector<std::unique_ptr<ExprAST>> expr) :expr_(std::move(expr)) {}
    llvm::Value* generateCode(CodeGenerator& cg);
    std::vector<std::unique_ptr<ExprAST>>& instructions();

private:
    std::vector<std::unique_ptr<ExprAST>> expr_;
};

class IfExprAST :public ExprAST
{
public:
    IfExprAST(std::unique_ptr<ExprAST> condition,
        std::unique_ptr<BlockExprAST> then, std::unique_ptr<BlockExprAST> els)
        :ExprAST("void"), Cond(std::move(condition)),Then(std::move(then)),
        /*ElseIf(nullptr),*/ Else(std::move(els))
    { }
    //IfExprAST(std::unique_ptr<ExprAST> condition,
    //    std::unique_ptr<BlockExprAST> then, std::unique_ptr<ExprAST> elseif)
    //    :Cond(std::move(condition)), Then(std::move(then)),
    //    ElseIf(std::move(elseif)), Else(nullptr)
    //{
    //}
    IfExprAST(std::unique_ptr<ExprAST> condition,
        std::unique_ptr<BlockExprAST> then)
        :ExprAST("void"), Cond(std::move(condition)), Then(std::move(then)),
        /*ElseIf(nullptr),*/ Else(nullptr) {
    }
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<BlockExprAST> Then;
    std::unique_ptr<BlockExprAST> Else;
    //std::unique_ptr<BlockExprAST> Else;
};

class VariableDefAST:public ExprAST
{
public:
    VariableDefAST(const std::string& type_name, const std::string& var_name,
        std::unique_ptr<ExprAST> init_value);

    VariableDefAST(const std::string& type_name, const std::string& var_name);
    llvm::Value* generateCode(CodeGenerator& cg) override;
    std::string getVarType() { return typename_; }
    std::string getVarName() { return varname_; }
    void setTemplateArgs(std::vector<std::unique_ptr<ExprAST>> args)
    {
        template_args_ = std::move(args);
    }

    void setInitValue(std::unique_ptr<ExprAST> initVal)
    {
        init_value_ = std::move(initVal);
    }

private:
    std::string typename_;
    std::string varname_;
    std::vector<std::unique_ptr<ExprAST>> template_args_;
    std::unique_ptr<ExprAST> init_value_;
};

class ReturnAST:public ExprAST
{
public:
    ReturnAST(std::unique_ptr<ExprAST> returnValue);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    std::unique_ptr<ExprAST> ret_val_;
};

class BinaryExprAST:public ExprAST
{
public:
    BinaryExprAST(OperatorType op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs,const std::string& retType);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    OperatorType Op;
    std::unique_ptr<ExprAST> LHS, RHS;
    std::string LType, RType;
};

class ForExprAST:public ExprAST
{
public:
    ForExprAST(std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> cond,
        std::unique_ptr<ExprAST> end, std::unique_ptr<BlockExprAST> body)
        :ExprAST("void"), Start(std::move(start)),Cond(std::move(cond)),
         End(std::move(end)),Body(std::move(body))
    { }
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    std::unique_ptr<ExprAST> Start, Cond, End;
    std::unique_ptr<BlockExprAST> Body;
};

class CallExprAST:public ExprAST
{
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args, const std::string& t);
    llvm::Value* generateCode(CodeGenerator& cg) override; 
    void setThis(llvm::Value* This);
    const std::string& getName() { return Callee; }
private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    llvm::Value* thisPtr;
};

class PrototypeAST
{
    std::string Name;
    std::vector<Variable> Args;
    std::string ClassName;
public:
    PrototypeAST(const std::string& name, std::vector<Variable> Args)
        : Name(name), Args(std::move(Args)) {

    }
    ~PrototypeAST() {}
    const std::string& getName() const { return Name; }
    std::vector<Variable>& Arg()
    {
        return Args;
    }
    llvm::Function* generateCode(CodeGenerator& cg);
    void setClassName(const std::string& className) { this->ClassName = className; }
    std::string getClassName() { return ClassName; }
};

class FunctionAST
{
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<BlockExprAST> Body;
   // std::vector<std::unique_ptr<ExprAST>> Body;
    
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
        std::unique_ptr<BlockExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {
    }
    ~FunctionAST() {}
    llvm::Function* generateCode(CodeGenerator& cg);
    std::string getName() { return Proto->getName(); }
    void setClassName(const std::string& className) { Proto->setClassName(className); }
    std::string getClassName() { return Proto->getClassName();}
    std::vector<Variable>& Arg() {
        return Proto->Arg();
    }
    auto& insturctions()
    {
        return Body->instructions();
    }
};

class ClassAST
{
    Class c;
    //std::vector<std::unique_ptr<FunctionAST>> Body;

public:
    ClassAST(const Class& clas)
        : c(clas)
    {
    }

    llvm::StructType* generateCode(CodeGenerator& cg);
};

class MemberAccessAST:public ExprAST,public AllocAST
{
    std::unique_ptr<ExprAST> var;
    std::unique_ptr<ExprAST> member;
    OperatorType op;

public:
    MemberAccessAST(std::unique_ptr<ExprAST> Var, std::unique_ptr<ExprAST> Member,
                    OperatorType Op, const std::string& retType);
    llvm::Value* generateCode(CodeGenerator& cg) override;
};

class UnaryExprAST:public ExprAST,public AllocAST
{
    std::unique_ptr<ExprAST> expr;
    OperatorType op;
    std::vector<std::unique_ptr<ExprAST>> args;

public:
    UnaryExprAST(std::unique_ptr<ExprAST> var,OperatorType Op, std::vector<std::unique_ptr<ExprAST>> Args)
        :ExprAST(""),expr(std::move(var)),op(Op),args(std::move(Args))
    { }
    UnaryExprAST(std::unique_ptr<ExprAST> var, OperatorType Op)
        :ExprAST(""),expr(std::move(var)), op(Op), args()
    { }
    llvm::Value* generateCode(CodeGenerator& cg) override;

};