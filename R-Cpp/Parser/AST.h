#pragma once
#include <memory>
#include <vector>
#include "../Util/Operator.h"
#include "../Util/Type.h"
#include <iostream>

namespace Parse
{
    class Decl
    {
    public:
        virtual void print(std::string indent, bool last)=0;
        virtual ~Decl() = default;
    };

    class Stmt
    {
    public:
        Stmt(const VarType& type):type_(type)
        { }

        Stmt()
        {}

        auto getType()
        {
            return type_;
        }

        virtual void print(std::string indent, bool last)=0;
        virtual ~Stmt() = default;
        VarType type_;
    };

    class Type
    {
        
    };

    class CompoundStmt:public Stmt
    {
    public:
        CompoundStmt(std::vector<std::unique_ptr<Stmt>> exprs);
        void print(std::string indent, bool last) override;

    private:
        std::vector<std::unique_ptr<Stmt>> stmts_;
    };

    class IfStmt :public Stmt
    {
    public:
        IfStmt(std::unique_ptr<Stmt> condition, std::unique_ptr<CompoundStmt> then,
               std::unique_ptr<CompoundStmt> els = nullptr);

        void print(std::string indent, bool last) override;
    private:
        std::unique_ptr<Stmt> cond_;
        std::unique_ptr<Stmt> then_;
        std::unique_ptr<Stmt> else_;
    };

    class ForStmt: public Stmt
    {
    public:
        ForStmt(std::unique_ptr<Stmt> start, std::unique_ptr<Stmt> cond, std::unique_ptr<Stmt> end,
                std::unique_ptr<CompoundStmt> body);

        void print(std::string indent, bool last) override;

    private:
        std::unique_ptr<Stmt> start_, cond_, end_;
        std::unique_ptr<CompoundStmt> body_;
    };

    class ReturnStmt: public Stmt
    {
    public:
        ReturnStmt(std::unique_ptr<Stmt> returnVal);
        void print(std::string indent, bool last) override;

    private:
        std::unique_ptr<Stmt> ret_val_;
    };

    class BinaryOperatorStmt :public Stmt
    {
    public:
        BinaryOperatorStmt(std::unique_ptr<Stmt> lhs, std::unique_ptr<Stmt> rhs, OperatorType op);
        void print(std::string indent, bool last) override;

    private:
        std::unique_ptr<Stmt> lhs_;
        std::unique_ptr<Stmt> rhs_;
        OperatorType op_;
    };

    class UnaryOperatorStmt :public Stmt
    {
    public:
        UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op);

        UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op, std::vector<std::unique_ptr<Stmt>> args);

        void print(std::string indent, bool last) override;

    private:
        std::unique_ptr<Stmt> stmt_;
        OperatorType op_;
        std::vector<std::unique_ptr<Stmt>> args_;
    };

    class VariableDefStmt: public Stmt
    {
    public:
        VariableDefStmt(const VarType& type, const std::string& name);

        void setInitValue(std::unique_ptr<Stmt> initVal);
        void print(std::string indent, bool last) override;
    private:
        VarType type_;
        std::string name_;
        std::unique_ptr<Stmt> init_val_;
    };

    class NamelessVariableStmt: public Stmt
    {
    public:
        NamelessVariableStmt(const VarType& type, std::vector<std::unique_ptr<Stmt>> args);

        void print(std::string indent, bool last) override;

    private:
        VarType type_;
        std::vector<std::unique_ptr<Stmt>> args_;
    };

    class VariableStmt: public Stmt
    {
    public:
        VariableStmt(const std::string& name);
        void print(std::string indent, bool last) override;

    private:
        std::string name_;
    };

    class IntegerStmt: public Stmt
    {
    public:
        IntegerStmt(std::int64_t val);
        void print(std::string indent, bool last) override;

    private:
        std::int64_t val_;
    };

    class FloatStmt: public Stmt
    {
    public:
        FloatStmt(double val);
        void print(std::string indent, bool last) override;

    private:
        double val_;
    };

    //class UsingStmt: public Stmt
    //{
    //    std::string new_name_;
    //    std::string
    //};

    class FunctionDecl:public Decl
    {
    public:
        FunctionDecl(::Function f, std::unique_ptr<CompoundStmt> body = nullptr);
        void print(std::string indent, bool last) override;
        ::Function getFunction();

    private:
        ::Function func_;
        std::unique_ptr<CompoundStmt> body_;
    };


    class ClassDecl:public Decl
    {
    public:
        ClassDecl(const std::string& name);

        void addMemberFunction(std::unique_ptr<FunctionDecl> func);
        void addMemberVariable(VarType type, const std::string name);
        void print(std::string indent, bool last) override;

    private:
        std::string name_;
        std::vector<std::pair<VarType, std::string>> member_variables_;
        std::vector<std::unique_ptr<FunctionDecl>> member_functions_;
    };


}
