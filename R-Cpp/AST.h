#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Token.h"
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

class VaribleExprAST:public ExprAST
{
public:
    VaribleExprAST(const std::string& n);
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

private:
    std::vector<std::unique_ptr<ExprAST>> expr_;
};

class IfExprAST :public ExprAST
{
public:
    IfExprAST(std::unique_ptr<ExprAST> condition,
        std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> els)
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
        std::unique_ptr<ExprAST> then)
        :Cond(std::move(condition)), Then(std::move(then)),
        /*ElseIf(nullptr),*/ Else(nullptr) {
    }
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<ExprAST> Then;
    std::unique_ptr<ExprAST> Else;
    //std::unique_ptr<BlockExprAST> Else;
};

class VaribleDefAST:public ExprAST
{
public:
    VaribleDefAST(const std::string& type_name, const std::string& var_name, std::unique_ptr<ExprAST> init_value);

    VaribleDefAST(const std::string& type_name, const std::string& var_name);
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
    BinaryExprAST(TokenType op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs);
    llvm::Value* generateCode(CodeGenerator& cg) override;

private:
    TokenType Op;
    std::unique_ptr<ExprAST> LHS, RHS;
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