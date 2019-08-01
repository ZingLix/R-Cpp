#pragma once
#include <memory>
#include "../CodeGenerator/AST.h"
#include "../Lexer/Lexer.h"
#include "SymbolTable.h"

namespace Parse {

    class Parser
    {
    public:
        Parser(const std::string& filename) :lexer_(filename),
            cur_token_(lexer_.nextToken()),
            symbol_(std::make_shared<Parse::SymbolTable>()),
            isExternal(false) {
        }

        std::unique_ptr<ExprAST> ParsePrimary();
        std::unique_ptr<ExprAST> ParseStatement();
        std::unique_ptr<ExprAST> ParseIntegerExpr();
        std::unique_ptr<ExprAST> ParseFloatExpr();
        std::unique_ptr<ExprAST> ParseParenExpr();
        std::unique_ptr<ExprAST> ParseIdentifierExpr();
        std::unique_ptr<ExprAST> ParseVariableDefinition(const std::string& type_name);
        std::unique_ptr<ExprAST> ParseReturnExpr();
        std::unique_ptr<ExprAST> ParseExpression();
        std::unique_ptr<ExprAST> ParseIfExpr();
        std::unique_ptr<ExprAST> ParseForExpr();
        std::unique_ptr<ExprAST> ParsePostOperator(std::unique_ptr<ExprAST> lhs);
        std::unique_ptr<ExprAST> ParseMemberAccess(std::unique_ptr<ExprAST> lhs, OperatorType Op);
        ::Function ParsePrototype();
        ::Function ParseFunction();
        std::unique_ptr<BlockExprAST> ParseBlock();
        std::unique_ptr<ClassAST> ParseClass(std::string className="");
        std::vector<std::unique_ptr<ExprAST>> ParseParenExprList();
        std::vector<std::unique_ptr<ExprAST>> ParseSquareExprList();
        std::vector<VarType> ParseAngleExprList();

        void HandleDefinition();
        void HandleClass();
        void MainLoop();
        std::vector<std::unique_ptr<FunctionAST>>& AST();
        std::vector<std::unique_ptr<ClassAST>>& Classes();
        std::vector<std::unique_ptr<PrototypeAST>>& Prototypes();
        std::shared_ptr<Parse::SymbolTable> symbolTable();

        void ParseExternal();
        void ParseInternal();
        void ParseUsing();
        void ParseTemplateClass();

        Class InstantiateTemplate(VarType name, std::vector<VarType> argList);

    private:
        Token& getNextToken();
        OperatorType getNextBinOperator();
        OperatorType getNextUnaryOperator();
        void error(const std::string& errmsg);
        std::unique_ptr<ExprAST> MergeExpr(std::unique_ptr<ExprAST>, std::unique_ptr<ExprAST>, OperatorType);
        std::vector<std::unique_ptr<ExprAST>> ParseExprList(TokenType endToken);
        VarType ParseType();
        Function generateFunction_new(const VarType& t);
        bool isPostOperator();
        void setExtraTokenStream(std::vector<Token>&& tokens) {
            extra_token_stream_index_ = 0;
            extra_token_stream_ = std::move(tokens);
        }

        void unsetExtraTokenStream() {
            extra_token_stream_.clear();
        }

        Lexer lexer_;
        Token cur_token_;
        std::vector<Token> extra_token_stream_;
        size_t extra_token_stream_index_;
        std::vector<std::unique_ptr<PrototypeAST>> proto_;
        std::vector<std::unique_ptr<FunctionAST>> expr_;
        std::vector<std::unique_ptr<ClassAST>> class_;
        std::shared_ptr<Parse::SymbolTable> symbol_;
        std::vector<std::string> cur_namespace_;
        VarType cur_class_;
        bool isExternal;
    };
}