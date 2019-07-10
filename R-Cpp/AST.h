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
    virtual ~ExprAST() = default;
    virtual llvm::Value* generateCode(CodeGenerator& cg) =0;
};

class IntegerExprAST:public ExprAST
{
public:
    IntegerExprAST(int v);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    int val;
};

class FloatExprAST :public ExprAST
{
public:
    FloatExprAST(float v);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    float val;
};

class VariableExprAST:public ExprAST
{
public:
    VariableExprAST(const std::string& n);
    llvm::Value* generateCode(CodeGenerator& cg) override;
    std::string getName() { return name; }
private:
    std::string name;
};

class BlockExprAST:public ExprAST
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
        :Cond(std::move(condition)),Then(std::move(then)),
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
        :Cond(std::move(condition)), Then(std::move(then)),
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
    VariableDefAST(const std::string& type_name, const std::string& var_name, std::unique_ptr<ExprAST> init_value);

    VariableDefAST(const std::string& type_name, const std::string& var_name);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    std::string typename_;
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
    BinaryExprAST(OperatorType op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs);
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
        :Start(std::move(start)),Cond(std::move(cond)),
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
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args);
    llvm::Value* generateCode(CodeGenerator& cg) override; 

private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
};

class PrototypeAST
{
    std::string Name;
    std::vector<std::pair<std::string,std::string>> Args;
    
public:
    PrototypeAST(const std::string& name, std::vector<std::pair<std::string, std::string>> Args)
        : Name(name), Args(std::move(Args)) {
    }
    ~PrototypeAST() {}
    const std::string& getName() const { return Name; }
    std::vector<std::pair<std::string, std::string>>& Arg()
    {
        return Args;
    }
    llvm::Function* generateCode(CodeGenerator& cg);
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
};