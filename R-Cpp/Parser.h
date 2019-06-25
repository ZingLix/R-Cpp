#pragma once
#include "AST.h"
#include "Lexer.h"
#include <memory>
#include <unordered_map>
#include <map>
#include <iostream>

class Parser
{
public:
    Parser(const std::string& filename):lexer_(filename),cur_token_(lexer_.nextToken())
    {}

    std::unique_ptr<ExprAST> ParseIntegerExpr() {
        auto res = std::make_unique<IntegerExprAST>(std::stoi(cur_token_.content));
        return res;
    }

    std::unique_ptr<ExprAST> ParseFloatExpr() {
        auto res = std::make_unique<FloatExprAST>(std::stof(cur_token_.content));
        return res;
    }

    std::unique_ptr<ExprAST> ParseParenExpr() {
        // ( expression )
        getNextToken();  //eat (
        auto expr = ParseExpression();
        if (!expr) return nullptr;
        if (cur_token_.type != TokenType::rParenthesis) return LogError("Expected ).");
        getNextToken();  //eat )
        return expr;
    }

    std::unique_ptr<ExprAST> ParseIdentifierExpr() {
        auto idname = cur_token_.content;
        getNextToken();
        if(cur_token_.type!=TokenType::lParenthesis) {
            return std::make_unique<VaribleExprAST>(idname);
        }
        getNextToken();
        std::vector<std::unique_ptr<ExprAST>> args;
        if(cur_token_.type!=TokenType::rParenthesis) {
            while (true) {
                if(auto arg=ParseExpression()) {
                    args.push_back(std::move(arg));
                }else {
                    return nullptr;
                }
                if(cur_token_.type==TokenType::rParenthesis) break;
                if (cur_token_.type != TokenType::Comma) return LogError("Expected , or ).");
                getNextToken();
            }
        }
        getNextToken();
        return std::make_unique<CallExprAST>(idname, std::move(args));
    }

    std::unique_ptr<ExprAST> ParsePrimary() {

        switch (cur_token_.type) {
        case TokenType::Identifier:
            return ParseIdentifierExpr();
        case TokenType::Integer:
            return ParseIntegerExpr();
        case TokenType::Float:
            return ParseFloatExpr();
        case TokenType::lParenthesis:
            return ParseParenExpr();
        default:
            return LogError("Unexpected token.");
        }

    }
    int GetTokPrecedence() {
        auto it = BinopPrecedence.find(cur_token_.type);
        if (it == BinopPrecedence.end()) return -1;

        // Make sure it's a declared binop.
        int TokPrec = it->second;
        if (TokPrec <= 0) return -1;
        return TokPrec;
    }

    std::unique_ptr<ExprAST> ParseExpression() {
        auto LHS = ParsePrimary();
        if (!LHS)
            return nullptr;

        return ParseBinOpRHS(0, std::move(LHS));
    }

    std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
        std::unique_ptr<ExprAST> LHS) {
        while (true) {
            int TokPrec = GetTokPrecedence();
            if (TokPrec < ExprPrec)
                return LHS;
            TokenType BinOp = cur_token_.type;
            getNextToken(); // eat binop
            auto RHS = ParsePrimary();
            if (!RHS)
                return nullptr;
            int NextPrec = GetTokPrecedence();
            if (TokPrec < NextPrec) {
                RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
                if (!RHS)
                    return nullptr;
            }
            LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                std::move(RHS));
        }
    }

    std::unique_ptr<PrototypeAST> ParsePrototype() {
        if (cur_token_.type != TokenType::Identifier)
            return LogErrorP("Expected function name in prototype");

        std::string FnName = cur_token_.content;
        getNextToken();

        if (cur_token_.type != TokenType::lParenthesis)
            return LogErrorP("Expected '(' in prototype");

        // Read the list of argument names.
        std::vector<std::string> ArgNames;
        while (getNextToken().type == TokenType::Identifier){
            ArgNames.push_back(cur_token_.content);
            getNextToken();
            if(cur_token_.type==TokenType::rParenthesis) break;
            if (cur_token_.type != TokenType::Comma)
                return LogErrorP("Expected ,");
        }

        if (cur_token_.type != TokenType::rParenthesis)
            return LogErrorP("Expected ')' in prototype");

        // success.
        getNextToken();  // eat ')'.

        return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
    }

    std::unique_ptr<FunctionAST> ParseDefinition() {
        getNextToken();  // eat fn.
        auto Proto = ParsePrototype();
        if (!Proto) return nullptr;

        if (auto E = ParseExpression())
            return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
        return nullptr;
    }

    std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
        if (auto E = ParseExpression()) {
            // Make an anonymous proto.
            auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
            return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
        }
        return nullptr;
    }
    void HandleDefinition() {
        if (ParseDefinition()) {
            fprintf(stderr, "Parsed a function definition.\n");
        } else {
            // Skip token for error recovery.
            getNextToken();
        }
    }
    void HandleTopLevelExpression() {
        // Evaluate a top-level expression into an anonymous function.
        if (ParseTopLevelExpr()) {
            fprintf(stderr, "Parsed a top-level expr\n");
        } else {
            // Skip token for error recovery.
            getNextToken();
        }
    }

    void MainLoop() {
        while (1) {
            switch (cur_token_.type) {
            case TokenType::Eof:
                return;
            case TokenType::Semicolon: // ignore top-level semicolons.
                getNextToken();
                break;
            case TokenType::Function:
                HandleDefinition();
                break;
            default:
                HandleTopLevelExpression();
                break;
            }
        }

    }
private:
    std::unique_ptr<ExprAST> LogError(const std::string& errmsg) {
        std::cout << errmsg << std::endl;
        return nullptr;
    }

    std::unique_ptr<PrototypeAST> LogErrorP(const std::string& errmsg) {
        std::cout << errmsg << std::endl;
        return nullptr;
    }

    Token& getNextToken() {
         return cur_token_ = lexer_.nextToken();
    }

    

    const std::map<TokenType, int> BinopPrecedence
    {
        {TokenType::lAngle,10},
        {TokenType::Plus,20},
        {TokenType::Minus,20},
        {TokenType::Multiply,40}
    };
    Lexer lexer_;
    Token cur_token_;
};
