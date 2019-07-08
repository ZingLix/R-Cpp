#pragma once
#include "AST.h"
#include "Lexer.h"
#include <memory>
#include <unordered_map>
#include <map>

class Parser
{
public:
    Parser(const std::string& filename):lexer_(filename),cur_token_(lexer_.nextToken())
    {}

    std::unique_ptr<ExprAST> ParsePrimary();
    std::unique_ptr<ExprAST> ParseStatement();
    std::unique_ptr<ExprAST> ParseIntegerExpr();
    std::unique_ptr<ExprAST> ParseFloatExpr();
    std::unique_ptr<ExprAST> ParseParenExpr();
    std::unique_ptr<ExprAST> ParseIdentifierExpr();
    std::unique_ptr<ExprAST> ParseVaribleDefinition(const std::string& type_name);
    std::unique_ptr<ExprAST> ParseReturnExpr();
    std::unique_ptr<ExprAST> ParseExpression();
    std::unique_ptr<ExprAST> ParseIfExpr();
    std::unique_ptr<ExprAST> ParseForExpr();
//    std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
//                                           std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<PrototypeAST> ParsePrototype();
    std::unique_ptr<FunctionAST> ParseDefinition();
    std::unique_ptr<BlockExprAST> ParseBlock();

    //std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    //    if (auto E = ParseExpression()) {
    //        // Make an anonymous proto.
    //        auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    //        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    //    }
    //    return nullptr;
    //}

    void HandleDefinition();

    //void HandleTopLevelExpression() {
    //    // Evaluate a top-level expression into an anonymous function.
    //    if (ParseTopLevelExpr()) {
    //        fprintf(stderr, "Parsed a top-level expr\n");
    //    } else {
    //        // Skip token for error recovery.
    //        getNextToken();
    //    }
    //}

    void MainLoop();

    std::vector<std::unique_ptr<FunctionAST>>& AST();

private:
    std::unique_ptr<ExprAST> LogError(const std::string& errmsg);
    std::unique_ptr<PrototypeAST> LogErrorP(const std::string& errmsg);
    Token& getNextToken();
    OperatorType getNextBinOperator();

    Lexer lexer_;
    Token cur_token_;
    std::vector<std::unique_ptr<FunctionAST>> expr_;
};
