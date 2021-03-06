#pragma once
#include <memory>
#include "../Lexer/Lexer.h"
#include "SymbolTable.h"
#include "AST.h"
#include "Type.h"

namespace Parse {

    class Parser
    {
    public:
        Parser(const std::string& filename) :lexer_(filename),isExternal(false)//,
            /*,
            isExternal(false),nameless_var_count_(0)*/ {
        }

        void print();
        void dumpToXML(const std::string& filename = "ast.xml");
        void convertToLLVM();
        ASTContext& context();
        void MainLoop();

    private:
        std::unique_ptr<Stmt> ParsePrimary();
        std::unique_ptr<Stmt> ParseStatement();
        std::unique_ptr<Stmt> ParseIntegerExpr();
        std::unique_ptr<Stmt> ParseFloatExpr();
        std::unique_ptr<Stmt> ParseParenExpr();
        std::unique_ptr<Stmt> ParseIdentifierExpr();
        std::unique_ptr<Stmt> ParseVariableDefinition(const std::string& type_name);
        std::unique_ptr<Stmt> ParseReturnExpr();
        std::unique_ptr<Stmt> ParseExpression();
        std::unique_ptr<Stmt> ParseIfExpr();
        std::unique_ptr<Stmt> ParseForExpr();
        std::unique_ptr<Stmt> ParsePostOperator(std::unique_ptr<Stmt> lhs);
        //std::unique_ptr<Stmt> ParseMemberAccess(std::unique_ptr<Stmt> lhs, OperatorType Op);
        std::unique_ptr<FunctionDecl> ParsePrototype();
        std::unique_ptr<FunctionDecl> ParseFunction();
        std::unique_ptr<CompoundStmt> ParseBlock();
        std::unique_ptr<ClassDecl> ParseClass();
        std::vector<std::unique_ptr<Stmt>> ParseParenExprList();
        std::vector<std::unique_ptr<Stmt>> ParseSquareExprList();
        std::vector<std::unique_ptr<Stmt>> ParseAngleExprList();
        std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> ParseFunctionArgList();
        void HandleDefinition();
        void HandleClass();

        void ParseExternal();
        void ParseInternal();
        //void ParseUsing();
        void ParseTemplateClass();

        Token& getNextToken();
        OperatorType getNextBinOperator();
        OperatorType getNextUnaryOperator();
        void error(const std::string& errmsg);
        std::unique_ptr<Stmt> MergeExpr(std::unique_ptr<Stmt>, std::unique_ptr<Stmt>, OperatorType);
        std::vector<std::unique_ptr<Stmt>> ParseExprList(TokenType endToken);
        std::unique_ptr<Stmt> ParseType();
        void generateNewForClass(Type* type); 
        //void generateDestructor(Class&c, std::unique_ptr<BlockExprAST> block);
        bool isPostOperator();

        Lexer lexer_;
        std::vector<std::unique_ptr<FunctionDecl>> functionDecls_;
        std::vector<std::unique_ptr<ClassDecl>> classDecls_;
        bool isExternal;
        ASTContext context_;
    };
}


