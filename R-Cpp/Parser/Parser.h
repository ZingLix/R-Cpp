#pragma once
#include <memory>
#include "../CodeGenerator/AST.h"
#include "../Lexer/Lexer.h"
#include "SymbolTable.h"
#include "AST.h"

namespace Parse {

    class Parser
    {
    public:
        Parser(const std::string& filename) :lexer_(filename),
            cur_token_(lexer_.nextToken()),cur_token_bak(cur_token_),
            symbol_(std::make_shared<Parse::SymbolTable>(*this)),
            isExternal(false),nameless_var_count_(0) {
        }

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
        ::Function ParsePrototype();
        ::Function ParseFunction();
        std::unique_ptr<CompoundStmt> ParseBlock();
        std::unique_ptr<ClassDecl> ParseClass(VarType classType=VarType());
        std::vector<std::unique_ptr<Stmt>> ParseParenExprList();
        std::vector<std::unique_ptr<Stmt>> ParseSquareExprList();
        std::vector<VarType> ParseAngleExprList();

        void HandleDefinition();
        void HandleClass();
        void MainLoop();
        std::shared_ptr<Parse::SymbolTable> symbolTable();

        void ParseExternal();
        void ParseInternal();
        void ParseUsing();
        void ParseTemplateClass();

        Class InstantiateTemplate(VarType type, const ClassTemplate& template_);
        void print();
        //std::unique_ptr<Stmt> callDestructor(const Variable& var);
        
    private:
        Token& getNextToken();
        OperatorType getNextBinOperator();
        OperatorType getNextUnaryOperator();
        void error(const std::string& errmsg);
        std::unique_ptr<Stmt> MergeExpr(std::unique_ptr<Stmt>, std::unique_ptr<Stmt>, OperatorType);
        std::vector<std::unique_ptr<Stmt>> ParseExprList(TokenType endToken);
        VarType ParseType();
        ::Function generateFunction_new(const VarType& t);
        //void generateDestructor(Class&c, std::unique_ptr<BlockExprAST> block);
        bool isPostOperator();

        Lexer lexer_;
        Token cur_token_,cur_token_bak;
        std::vector<std::unique_ptr<Decl>> declartions_;
        std::shared_ptr<Parse::SymbolTable> symbol_;
        std::vector<std::string> cur_namespace_;
        VarType cur_class_;
        bool isExternal;
        int32_t nameless_var_count_;
    };
}
