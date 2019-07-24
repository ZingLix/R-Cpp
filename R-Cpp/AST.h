#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Operator.h"
#include <llvm/IR/Value.h>
#include "CodeGenerator.h"
#include "Type.h"

class ExprAST
{
public:
    ExprAST(const VarType& s) :type(s) {}
    VarType getType()
    {
        return type;
    }
    std::string getTypeName()
    {
        return type.typeName;
    }
    virtual ~ExprAST() = default;
    virtual llvm::Value* generateCode(CodeGenerator& cg) =0;
protected:
    VarType type;
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
    VariableExprAST(const std::string& n, const VarType& t);
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
        :ExprAST(VarType("null")), Cond(std::move(condition)),Then(std::move(then)),
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
        :ExprAST(VarType("null")), Cond(std::move(condition)), Then(std::move(then)),
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
    VariableDefAST(const VarType& type_name, const std::string& var_name,
        std::unique_ptr<ExprAST> init_value);

    VariableDefAST(const VarType& type_name, const std::string& var_name);
    llvm::Value* generateCode(CodeGenerator& cg) override;
    VarType getVarType() { return type_; }
    std::string getVarName() { return varname_; }
    void setTemplateArgs(std::vector<VarType> args)
    {
        type_.templateArgs = args;
    }

    void setInitValue(std::unique_ptr<ExprAST> initVal)
    {
        init_value_ = std::move(initVal);
    }

private:
    VarType type_;
    std::string varname_;
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
};

class ForExprAST:public ExprAST
{
public:
    ForExprAST(std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> cond,
        std::unique_ptr<ExprAST> end, std::unique_ptr<BlockExprAST> body)
        :ExprAST(VarType("null")), Start(std::move(start)),Cond(std::move(cond)),
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
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args, const VarType& t);
    llvm::Value* generateCode(CodeGenerator& cg) override; 
    void setThis(std::unique_ptr<ExprAST> This);
    const std::string& getName() { return Callee; }
private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    std::unique_ptr<ExprAST> thisPtr;
};

class PrototypeAST
{
    ::Function F;
public:
    PrototypeAST(const ::Function& func)
        : F(func) {

    }
    ~PrototypeAST() {}
    const std::string& getName() const { return F.name; }
    std::vector<Variable>& Arg()
    {
        return F.args;
    }
    llvm::Function* generateCode(CodeGenerator& cg);
    void setClassType(const VarType& className) { F.classType = className; }
    VarType getClassType() { return F.classType; }
    ::Function getFunction() { return F; }
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
    void setClassType(const VarType& className) { Proto->setClassType(className); }
    VarType getClassType() { return Proto->getClassType();}
    std::vector<Variable>& Arg() {
        return Proto->Arg();
    }
    auto& insturctions()
    {
        return Body->instructions();
    }
    ::Function getFunction() { return Proto->getFunction(); }
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
    std::string member;
    OperatorType op;

public:
    MemberAccessAST(std::unique_ptr<ExprAST> Var, std::string Member,
                    OperatorType Op, const VarType& retType);
    llvm::Value* generateCode(CodeGenerator& cg) override;
};

class UnaryExprAST:public ExprAST,public AllocAST
{
    std::unique_ptr<ExprAST> expr;
    OperatorType op;
    std::vector<std::unique_ptr<ExprAST>> args;

public:
    UnaryExprAST(std::unique_ptr<ExprAST> var,OperatorType Op, std::vector<std::unique_ptr<ExprAST>> Args, const VarType& type)
        :ExprAST(type),expr(std::move(var)),op(Op),args(std::move(Args))
    { }
    UnaryExprAST(std::unique_ptr<ExprAST> var, OperatorType Op,const VarType& type)
        :ExprAST(type),expr(std::move(var)), op(Op), args()
    { }
    llvm::Value* generateCode(CodeGenerator& cg) override;

};