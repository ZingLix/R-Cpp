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
        CompoundStmt(std::vector<std::unique_ptr<Stmt>> exprs)
            :stmts_(std::move( exprs))
        { }

        void print(std::string indent, bool last) override
        {
            indent += last ? "  " : "| ";
            for(size_t i=0;i<stmts_.size();++i)
            {
                stmts_[i]->print(indent, i == stmts_.size() - 1);
            }
        }
    private:
        std::vector<std::unique_ptr<Stmt>> stmts_;
    };

    class IfStmt :public Stmt
    {
    public:
        IfStmt(std::unique_ptr<Stmt> condition, std::unique_ptr<CompoundStmt> then, std::unique_ptr<CompoundStmt> els = nullptr)
            :cond_(std::move(condition)),then_(std::move(then)),else_(std::move(els))
        {}
        void print(std::string indent, bool last) override
        {
            std::cout<<indent << "+-IfStmt "<<std::endl;
            indent += last ? "  " : "| ";
            cond_->print(indent, false);
            then_->print(indent, else_ == nullptr);
            if(else_)
                else_->print(indent, true);
        }
    private:
        std::unique_ptr<Stmt> cond_;
        std::unique_ptr<Stmt> then_;
        std::unique_ptr<Stmt> else_;
    };

    class ForStmt: public Stmt
    {
    public:
        ForStmt(std::unique_ptr<Stmt> start, std::unique_ptr<Stmt> cond, std::unique_ptr<Stmt> end, std::unique_ptr<CompoundStmt> body)
            :start_(std::move(start)),cond_(std::move(cond)),end_(std::move(end)),body_(std::move(body))
        {
            
        }

        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-ForStmt" << std::endl;
            indent += last ? "  " : "| ";
            std::cout << indent << "+-start" << std::endl;
            start_->print(indent+" |",false);
            std::cout << indent << "+-cond" << std::endl;
            cond_->print(indent + " |", false);
            std::cout << indent << "+-body" << std::endl;
            body_->print(indent + " |", false);
            std::cout << indent << "+-end" << std::endl;
            end_->print(indent + "  ", true);
        }

    private:
        std::unique_ptr<Stmt> start_, cond_, end_;
        std::unique_ptr<CompoundStmt> body_;
    };

    class ReturnStmt: public Stmt
    {
    public:
        ReturnStmt(std::unique_ptr<Stmt> returnVal)
            :ret_val_(std::move(returnVal))
        { }
        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-ReturnStmt" << std::endl;
            indent += last?"  ": " |";
            ret_val_->print(indent, true);
        }
    private:
        std::unique_ptr<Stmt> ret_val_;
    };

    class BinaryOperatorStmt :public Stmt
    {
    public:
        BinaryOperatorStmt(std::unique_ptr<Stmt> lhs, std::unique_ptr<Stmt> rhs, OperatorType op)
            :lhs_(std::move(lhs)),rhs_(std::move(rhs)),op_(op)
        { }
        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-BinaryOperatorStmt "<<static_cast<int>(op_) << std::endl;
            indent += last ? "  " : "| ";
            lhs_->print(indent, false);
            rhs_->print(indent, true);
        }
    private:
        std::unique_ptr<Stmt> lhs_;
        std::unique_ptr<Stmt> rhs_;
        OperatorType op_;
    };

    class UnaryOperatorStmt :public Stmt
    {
    public:
        UnaryOperatorStmt(std::unique_ptr<Stmt> expr,OperatorType op)
            :stmt_(std::move(expr)),op_(op)
        { }
        UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op, std::vector<std::unique_ptr<Stmt>> args)
            :stmt_(std::move(expr)), op_(op), args_(std::move(args))
        { }

        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-UnaryOperatorStmt "<<static_cast<int>(op_) << std::endl;
            indent += last ? "  " : "| ";
            stmt_->print(indent, true);
            indent += " |";
            for(size_t i=0;i<args_.size();++i)
            {
                //args_[i]->print(indent, i == args_.size() - 1);
            }
        }

    private:
        std::unique_ptr<Stmt> stmt_;
        OperatorType op_;
        std::vector<std::unique_ptr<Stmt>> args_;
    };

    class VariableDefStmt: public Stmt
    {
    public:
        VariableDefStmt(const VarType& type, const std::string& name)
            :type_(type),name_(name)
        { }
        void setInitValue(std::unique_ptr<Stmt> initVal)
        {
            init_val_ = std::move(initVal);
        }

        void print(std::string indent, bool last) override
        {
            std::cout<<indent << "+-VariableDefStmt " << name_ << std::endl;
            indent += last ? "  " : "| ";
            if (init_val_)
                init_val_->print(indent, true);
        }
    private:
        VarType type_;
        std::string name_;
        std::unique_ptr<Stmt> init_val_;
    };

    class NamelessVariableStmt: public Stmt
    {
    public:
        NamelessVariableStmt(const VarType& type, std::vector<std::unique_ptr<Stmt>> args)
            :type_(type),args_(std::move(args))
        { }

        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-NamelessVariableStmt " << type_.typeName << std::endl;
            indent += last ? "  " : "| ";
            for (size_t i = 0; i < args_.size(); ++i) {
                args_[i]->print(indent, i == args_.size() - 1);
            }
        }

    private:
        VarType type_;
        std::vector<std::unique_ptr<Stmt>> args_;
    };

    class VariableStmt: public Stmt
    {
    public:
        VariableStmt(const std::string& name):name_(name)
        { }
        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-VariableStmt " << name_ << std::endl;
        }
    private:
        std::string name_;
    };

    class IntegerStmt: public Stmt
    {
    public:
        IntegerStmt(std::int64_t val):Stmt(VarType("i32")), val_(val)
        { }
        void print(std::string indent, bool last) override {
            std::cout << indent << "+-IntegerStmt " << val_ << std::endl;
        }
    private:
        std::int64_t val_;
    };

    class FloatStmt: public Stmt
    {
    public:
        FloatStmt(double val):Stmt(VarType("double")), val_(val)
        { }
        void print(std::string indent, bool last) override {
            std::cout << indent << "+-FloatStmt " << val_ << std::endl;
        }
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
        FunctionDecl(::Function f, std::unique_ptr<CompoundStmt> body=nullptr)
            :func_(f),body_(std::move(body))
        { }

        void print(std::string indent, bool last) override
        {
            std::cout <<indent << "+-Function " << func_.name << std::endl;
            //indent += last ? "  " : "| ";
            body_->print(indent, true);
        }

    private:
        ::Function func_;
        std::unique_ptr<CompoundStmt> body_;
    };


    class ClassDecl:public Decl
    {
    public:
        ClassDecl(const std::string& name):name_(name)
        { }

        void addMemberFunction(std::unique_ptr<FunctionDecl> func)
        {
            member_functions_.emplace_back(std::move(func));
        }
        void addMemberVariable(VarType type, const std::string name)
        {
            member_variables_.emplace_back(type, name);
        }
        void print(std::string indent, bool last) override
        {
            std::cout << indent << "+-ClassDecl "<<name_ << std::endl;
            indent += last ? "  " : "| ";
            for(size_t i=0;i<member_functions_.size();++i)
            {
                member_functions_[i]->print(indent, i == member_functions_.size() - 1);
            }
        }
    private:
        std::string name_;
        std::vector<std::pair<VarType, std::string>> member_variables_;
        std::vector<std::unique_ptr<FunctionDecl>> member_functions_;
    };
}
