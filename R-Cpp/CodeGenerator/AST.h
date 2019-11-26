#pragma once
#include <string>
#include <memory>
#include <vector>
#include "../Util/Operator.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include "../Parser/Type.h"

class ExprAST
{
public:
    ExprAST(const std::string& s) :type(s) {}
    std::string getType()
    {
        return type;
    }
    void setType(const std::string& t) {
        type = t;
    }
    virtual ~ExprAST() = default;
    virtual llvm::Value* generateCode(CG::CodeGenerator& cg) =0;
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
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
    std::int64_t value() { return val; }
private:
    std::int64_t val;
};

class FloatExprAST :public ExprAST
{
public:
    FloatExprAST(double v);
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;

private:
    double val;
};

class VariableExprAST:public ExprAST,public AllocAST
{
public:
    VariableExprAST(const std::string& n, const std::string& t);
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
    std::string getName() { return name; }

private:
    std::string name;

};

class NamelessVarExprAST:public ExprAST,public AllocAST
{
public:
    NamelessVarExprAST(const std::string Name,const std::string& Type, const std::string& Constructor,std::vector<std::unique_ptr<ExprAST>> Args)
        :ExprAST(Type), name(Name),constructor(Constructor),args(std::move(Args))
    { }
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
private:
    std::string name;
    std::string constructor;
    std::vector<std::unique_ptr<ExprAST>> args;
};

class BlockExprAST :public ExprAST
{
public:
    BlockExprAST(std::vector<std::unique_ptr<ExprAST>> expr, bool hasReturnStatement ,const std::string& t="void")
        :ExprAST(t), expr_(std::move(expr)),hasReturn_(hasReturnStatement) {}
    llvm::Value* generateCode(CG::CodeGenerator& cg);
    std::vector<std::unique_ptr<ExprAST>>& instructions();
    bool hasReturn() { return hasReturn_; }
    void setHasReturn() { hasReturn_ = true; }

private:
    std::vector<std::unique_ptr<ExprAST>> expr_;
    bool hasReturn_;
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
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;

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
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
    std::string getVarName() { return varname_; }
    void setTemplateArgs(std::vector<std::string> args)
    {
        templateArgs = std::move(args);
    }

    void setInitValue(std::unique_ptr<ExprAST> initVal)
    {
        init_value_ = std::move(initVal);
    }

private:
    std::string type_;
    std::vector<std::string> templateArgs;
    std::string varname_;
    std::unique_ptr<ExprAST> init_value_;
};

class ReturnAST:public ExprAST
{
public:
    ReturnAST(std::unique_ptr<ExprAST> returnValue, std::vector<std::unique_ptr<ExprAST>> destructorExpr);
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
    
private:
    std::unique_ptr<ExprAST> ret_val_;
    std::vector<std::unique_ptr<ExprAST>> destructor_expr_;
};

class BinaryExprAST:public ExprAST
{
public:
    BinaryExprAST(OperatorType op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs,const std::string& retType);
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;

private:
    OperatorType Op;
    std::unique_ptr<ExprAST> LHS, RHS;
};

class ForExprAST:public ExprAST
{
public:
    ForExprAST(std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> cond,
        std::unique_ptr<ExprAST> end, std::unique_ptr<BlockExprAST> body)
        :ExprAST("null"), Start(std::move(start)),Cond(std::move(cond)),
         End(std::move(end)),Body(std::move(body))
    { }
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;

private:
    std::unique_ptr<ExprAST> Start, Cond, End;
    std::unique_ptr<BlockExprAST> Body;
};

class CallExprAST:public ExprAST
{
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args, const std::string& t);
    llvm::Value* generateCode(CG::CodeGenerator& cg) override; 
    void setThis(std::unique_ptr<ExprAST> This);
    const std::string& getName() { return Callee; }
    void setName(const std::string& name) { Callee = name; }
    void setArgs(std::vector<std::unique_ptr<ExprAST>> args) { Args = std::move(args); }
private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    std::unique_ptr<ExprAST> thisPtr;
};

class PrototypeAST
{
    std::string name_;
    std::vector<std::pair<std::string,std::string>> arg_list_;
    std::string return_type_;
    std::string class_type_;

public:
    PrototypeAST(const std::string& name, std::vector<std::pair<std::string, std::string>> argList,std::string returnType,std::string classType="")
        : name_(name), arg_list_(std::move(argList)),return_type_(returnType),class_type_(classType)
    { }
    ~PrototypeAST() {}
    const std::string& name() const { return name_; }
    const std::vector<std::pair<std::string, std::string>>& Arg()
    {
        return arg_list_;
    }
    llvm::Function* generateCode(CG::CodeGenerator& cg);
    void setClassType(const std::string& classType) { class_type_ = classType; }
    std::string getClassType() { return class_type_; }
};

class FunctionAST
{
    //::Function Proto;
    std::string function_name_;
    std::unique_ptr<BlockExprAST> body_;
    std::string class_name_;
   // std::vector<std::unique_ptr<ExprAST>> Body;
    
public:
    FunctionAST(const std::string& name,
        std::unique_ptr<BlockExprAST> Body,const std::string& className="")
        : function_name_(name), body_(std::move(Body)),class_name_(className) {
    }
    ~FunctionAST() {}
    llvm::Function* generateCode(CG::CodeGenerator& cg);
    //std::string getName() { return Proto.name; }
    //VarType getClassType() { return Proto.classType;}
    auto& insturctions()
    {
        return body_->instructions();
    }
    //::Function getFunction() { return Proto; }
};

class ClassAST
{
    std::string name_;
    std::vector<std::pair<std::string, std::string>> members_;
    Parse::Type* type_;

public:
    ClassAST(const std::string& name, std::vector<std::pair<std::string, std::string>> members,
        Parse::Type* type)
        : name_(name),members_(std::move(members)),type_(type)
    {
    }

    llvm::StructType* generateCode(CG::CodeGenerator& cg);
    llvm::Value* generateFunction_new(CG::CodeGenerator& cg);
};

class MemberAccessAST:public ExprAST,public AllocAST
{
    std::unique_ptr<ExprAST> var;
    int memberIndex;

public:
    MemberAccessAST(std::unique_ptr<ExprAST> Var, int index,
                    const std::string& retType);
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
};

class UnaryExprAST:public ExprAST,public AllocAST
{
    std::unique_ptr<ExprAST> expr;
    OperatorType op;
    std::vector<std::unique_ptr<ExprAST>> args;

public:
    UnaryExprAST(std::unique_ptr<ExprAST> var,OperatorType Op, std::vector<std::unique_ptr<ExprAST>> Args, const std::string& type)
        :ExprAST(type),expr(std::move(var)),op(Op),args(std::move(Args))
    { }
    UnaryExprAST(std::unique_ptr<ExprAST> var, OperatorType Op,const std::string& type)
        :ExprAST(type),expr(std::move(var)), op(Op), args()
    { }
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
};

class NamespaceExprAST:public ExprAST
{
    std::string name_;
public:
    NamespaceExprAST(std::string name):ExprAST("null"),name_(name){}
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
    std::string getName() { return name_; }
};

class NonExprAST :public ExprAST
{
public:
    NonExprAST() :ExprAST("null") {}
    llvm::Value* generateCode(CG::CodeGenerator& cg) override;
};
