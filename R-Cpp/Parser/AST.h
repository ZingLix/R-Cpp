#pragma once
#include <memory>
#include <vector>
#include "../Util/Operator.h"
#include "../CodeGenerator/AST.h"
#include "ASTContext.h"

namespace Parse
{
    class Decl
    {
    public:
        virtual void print(std::string indent, bool last)=0;
        virtual std::string dumpToXML() const = 0;
        virtual ~Decl() = default;
    };

    class Stmt
    {
    public:
        Stmt()
        {}

        auto getType()
        {
            return type_;
        }

        void setType(Type* t) {
            type_ = t;
        }

        virtual void print(std::string indent, bool last)=0;
        virtual std::string dumpToXML() const = 0;
        virtual std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) = 0;
        virtual ~Stmt() {
            //assert(type_!=nullptr);
        }
        Type* type_;
    };

    class CompoundStmt:public Stmt
    {
    public:
        CompoundStmt(std::vector<std::unique_ptr<Stmt>> exprs);
        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
        std::unique_ptr<BlockExprAST> toBlockExprAST(ASTContext*);
    private:
        std::vector<std::unique_ptr<Stmt>> stmts_;
    };

    class IfStmt :public Stmt
    {
    public:
        IfStmt(std::unique_ptr<Stmt> condition, std::unique_ptr<CompoundStmt> then,
               std::unique_ptr<CompoundStmt> els = nullptr);

        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext* context) override;
    private:
        std::unique_ptr<Stmt> cond_;
        std::unique_ptr<CompoundStmt> then_;
        std::unique_ptr<CompoundStmt> else_;
    };

    class ForStmt: public Stmt
    {
    public:
        ForStmt(std::unique_ptr<Stmt> start, std::unique_ptr<Stmt> cond, std::unique_ptr<Stmt> end,
                std::unique_ptr<CompoundStmt> body);

        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
    private:
        std::unique_ptr<Stmt> start_, cond_, end_;
        std::unique_ptr<CompoundStmt> body_;
    };

    class ReturnStmt: public Stmt
    {
    public:
        ReturnStmt(std::unique_ptr<Stmt> returnVal);
        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
    private:
        std::unique_ptr<Stmt> ret_val_;
    };

    class BinaryOperatorStmt :public Stmt
    {
    public:
        BinaryOperatorStmt(std::unique_ptr<Stmt> lhs, std::unique_ptr<Stmt> rhs, OperatorType op);
        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
        Type* getLHSType();
        Type* getRHSType();

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
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;

    private:
        std::unique_ptr<Stmt> stmt_;
        OperatorType op_;
        std::vector<std::unique_ptr<Stmt>> args_;
    };

    class VariableDefStmt: public Stmt
    {
    public:
        VariableDefStmt(std::unique_ptr<Stmt> type, const std::string& name);

        void setInitValue(std::unique_ptr<Stmt> initVal);
        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
    private:
        std::unique_ptr<Stmt> vartype_;
        std::string name_;
        std::unique_ptr<Stmt> init_val_;
    };

    class TypeStmt: public Stmt
    {
    public:
        TypeStmt(const std::string& name, std::vector<std::unique_ptr<Stmt>> arglist = {});
        void print(std::string indent, bool last) override;

        std::string getName();
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
    private:
        std::string name_;
        std::vector<std::unique_ptr<Stmt>> arglist_;
    };


    class VariableStmt: public Stmt
    {
    public:
        VariableStmt(const std::string& name);
        void print(std::string indent, bool last) override;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
        const std::string& getName();
    private:
        std::string name_;
    };

    class IntegerStmt: public Stmt
    {
    public:
        IntegerStmt(std::int64_t val);
        void print(std::string indent, bool last) override;
        std::int64_t getNumber() const;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
    private:
        std::int64_t val_;
    };

    class FloatStmt: public Stmt
    {
    public:
        FloatStmt(double val);
        void print(std::string indent, bool last) override;
        double getNumber()const;
        std::string dumpToXML() const override;
        std::unique_ptr<ExprAST> toLLVMAST(ASTContext*) override;
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
        FunctionDecl(std::string funcName,
        std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> args,
        std::unique_ptr<Stmt> retType ,
        std::unique_ptr<CompoundStmt> body = nullptr, bool isExternal=false);
        void print(std::string indent, bool last) override;
        void setBody(std::unique_ptr<CompoundStmt> body);
        std::string dumpToXML() const override;
        void toLLVM(ASTContext* context);
        FunctionType* registerPrototype(ASTContext* context);

    private:
        std::string funcName_;
        std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> args_;
        std::unique_ptr<Stmt> retType_;
        std::unique_ptr<CompoundStmt> body_;
        bool isExternal_;
        FunctionType* funcType_;
    };


    class ClassDecl:public Decl
    {
    public:
        ClassDecl(const std::string& name);

        void addMemberFunction(std::unique_ptr<FunctionDecl> func);
        void addMemberVariable(std::unique_ptr<Stmt> type, const std::string& name);
        void addConstructor(std::unique_ptr<FunctionDecl> func);
        void setDestructor(std::unique_ptr<FunctionDecl> func);
        void print(std::string indent, bool last) override;
        const std::string& name() { return name_; }
        std::string dumpToXML() const override;
        void toLLVM(ASTContext* context);
        const std::vector<std::pair<std::unique_ptr<Stmt>, std::string>>& getMemberVariables();
        void registerMemberFunction(ASTContext* context);
        void generateNewFunction(ASTContext* context);

    private:
        std::string name_;
        std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> memberVariables_;
        std::vector<std::unique_ptr<FunctionDecl>> memberFunctions_;
        std::vector<std::unique_ptr<FunctionDecl>> constructors_;
        std::unique_ptr<FunctionDecl> destructor_;
        CompoundType* classType_;
    };


}
