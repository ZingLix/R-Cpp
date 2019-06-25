#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Token.h"

class ExprAST
{
public:
    virtual ~ExprAST(){}
};

class IntegerExprAST:public ExprAST
{
public:
    IntegerExprAST(int v):val(v){}

private:
    int val;
};

class FloatExprAST :public ExprAST
{
public:
    FloatExprAST(float v) :val(v) {}

private:
    float val;
};

class VaribleExprAST:public ExprAST
{
public:
    VaribleExprAST(const std::string& n):name(n){}

private:
    std::string name;
};

class BinaryExprAST:public ExprAST
{
public:
    BinaryExprAST(TokenType op,std::unique_ptr<ExprAST> lhs,std::unique_ptr<ExprAST> rhs)
        :Op(op),LHS(std::move(lhs)),RHS(std::move(rhs))
    { }

private:
    TokenType Op;
    std::unique_ptr<ExprAST> LHS, RHS;
};

class CallExprAST:public ExprAST
{
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
        :Callee(callee),Args(std::move(args))
    { }

private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
};

class PrototypeAST
{
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string& name, std::vector<std::string> Args)
        : Name(name), Args(std::move(Args)) {
    }

    const std::string& getName() const { return Name; }
};

class FunctionAST
{
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
        std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {
    }
};