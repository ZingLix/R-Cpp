#include "Parser.h"
#include <iostream>

std::unique_ptr<ExprAST> Parser::ParseIntegerExpr() {
    auto res = std::make_unique<IntegerExprAST>(std::stoi(cur_token_.content));
    getNextToken();
    return res;
}

std::unique_ptr<ExprAST> Parser::ParseFloatExpr() {
    auto res = std::make_unique<FloatExprAST>(std::stof(cur_token_.content));
    getNextToken();
    return res;
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
    if (cur_token_.type == TokenType::Return) {
        return ParseReturnExpr();
    }
    if (cur_token_.type == TokenType::Identifier) {
        return ParseIdentifierExpr();
    }
    if (cur_token_.type == TokenType::Integer) {
        return ParseIntegerExpr();
    }
    if (cur_token_.type == TokenType::Float) {
        return ParseFloatExpr();
    }
    if (cur_token_.type == TokenType::lParenthesis) {
        return ParseParenExpr();
    }
    if (cur_token_.type == TokenType::If) {
        return ParseIfExpr();
    }
    if(cur_token_.type==TokenType::For) {
        return ParseForExpr();
    }
    return LogError("Unexpected token.");
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    // ( expression )
    getNextToken(); //eat (
    auto expr = ParseExpression();
    if (!expr) return nullptr;
    if (cur_token_.type != TokenType::rParenthesis) return LogError("Expected ).");
    getNextToken(); //eat )
    return expr;
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
    auto idname = cur_token_.content;
    getNextToken();
    if (cur_token_.type == TokenType::Identifier) {
        return ParseVaribleDefinition(idname);
    }
    if (cur_token_.type != TokenType::lParenthesis) {
        return std::make_unique<VariableExprAST>(idname);;
    }
    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> args;
    if (cur_token_.type != TokenType::rParenthesis) {
        while (true) {
            if (auto arg = ParseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }
            if (cur_token_.type == TokenType::rParenthesis) break;
            if (cur_token_.type != TokenType::Comma) return LogError("Expected , or ).");
            getNextToken();
        }
    }
    getNextToken();
    return std::make_unique<CallExprAST>(idname, std::move(args));
}

std::unique_ptr<ExprAST> Parser::ParseForExpr() {
    getNextToken(); //eat for
    if(cur_token_.type!=TokenType::lParenthesis) {
        return LogError("Expected (.");
    }
    getNextToken(); //eat (
    auto start = ParseExpression();
    if(cur_token_.type!=TokenType::Semicolon) {
        return LogError("Expected ;");
    }
    getNextToken(); //eat ;
    auto cond = ParseExpression();
    if (cur_token_.type != TokenType::Semicolon) {
        return LogError("Expected ;");
    }
    getNextToken(); //eat ;
    auto end = ParseExpression();
    if (cur_token_.type != TokenType::rParenthesis) {
        return LogError("Expected )");
    }
    getNextToken(); //eat )
    if(cur_token_.type!=TokenType::lBrace) {
        return LogError("Expected {");
    }
    auto body = ParseBlock();
    return std::make_unique<ForExprAST>(std::move(start), std::move(cond), std::move(end), std::move(body));
}


std::unique_ptr<ExprAST> Parser::ParseVaribleDefinition(const std::string& type_name) {
    auto varname = cur_token_.content;
    getNextToken();
    if (cur_token_.type == TokenType::Equal) {
        getNextToken();
        auto E = ParseExpression();
        return std::make_unique<VariableDefAST>(type_name, varname, std::move(E));
    }
    if (cur_token_.type == TokenType::Semicolon) {
        return std::make_unique<VariableDefAST>(type_name, varname);
    }
    return LogError("Expected expression or ;.");
}

std::unique_ptr<ExprAST> Parser::ParseReturnExpr() {
    getNextToken();
    auto retval = ParseExpression();
    if (retval != nullptr) return std::make_unique<ReturnAST>(std::move(retval));
    getNextToken();
    return LogError("Invalid return value.");
}

int Parser::GetTokPrecedence() {
    auto it = BinopPrecedence.find(cur_token_.type);
    if (it == BinopPrecedence.end()) return -1;
    // Make sure it's a declared binop.
    int TokPrec = it->second;
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
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

std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
    getNextToken();  // eat if
    auto cond = ParseExpression();
    if (!cond) return nullptr;
    if (cur_token_.type != TokenType::lBrace)
        return LogError("Expected {.");
    auto then = ParseBlock();
    if(cur_token_.type==TokenType::Else) {
        getNextToken();
        if (cur_token_.type == TokenType::If) {
            auto elseif = ParseIfExpr();
            if (!elseif) return nullptr;
            return std::make_unique<IfExprAST>(std::move(cond), std::move(then), std::move(elseif));
        }
        auto els = ParseBlock();
        if (!els) return nullptr;
        return std::make_unique<IfExprAST>(std::move(cond), std::move(then), std::move(els));
    }
    return std::make_unique<IfExprAST>(std::move(cond), std::move(then));
}

std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
    if (cur_token_.type != TokenType::Identifier)
        return LogErrorP("Expected function name in prototype");

    std::string FnName = cur_token_.content;
    getNextToken();

    if (cur_token_.type != TokenType::lParenthesis)
        return LogErrorP("Expected '(' in prototype");

    // Read the list of argument names.
    std::vector<std::pair<std::string, std::string>> ArgNames;
    while (getNextToken().type == TokenType::Identifier) {
        auto type_name = cur_token_.content;
        getNextToken();
        ArgNames.emplace_back(type_name, cur_token_.content);
        getNextToken();
        if (cur_token_.type == TokenType::rParenthesis) break;
        if (cur_token_.type != TokenType::Comma)
            return LogErrorP("Expected ,");
    }

    if (cur_token_.type != TokenType::rParenthesis)
        return LogErrorP("Expected ')' in prototype");

    // success.
    getNextToken(); // eat ')'.

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<BlockExprAST> Parser::ParseBlock() {
    getNextToken(); //eat {
    std::vector<std::unique_ptr<ExprAST>> body;
    while (cur_token_.type != TokenType::rBrace && cur_token_.type != TokenType::Eof) {
        auto E = ParseExpression();
        if (E != nullptr) body.emplace_back(std::move(E));
        if(cur_token_.type==TokenType::Semicolon)
        getNextToken(); // eat ;
    }
    if (cur_token_.type != TokenType::rBrace) {
        LogError("Expected }.");
        return nullptr;
    }
    getNextToken(); //eat }
    return std::make_unique<BlockExprAST>(std::move(body));
}

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
    getNextToken(); // eat fn.
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;
    if (cur_token_.type != TokenType::lBrace) {
        std::cout << "Expected {." << std::endl;
        return nullptr;
    }
    auto body = ParseBlock();
    if (!body) {
        return nullptr;
    }
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(body));
}

void Parser::HandleDefinition() {
    auto func = ParseDefinition();
    if (func != nullptr) {
        fprintf(stderr, "Parsed a function definition.\n");
        expr_.emplace_back(std::move(func));
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

void Parser::MainLoop() {
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
            getNextToken();
            //    HandleTopLevelExpression();
            break;
        }
    }
}

std::vector<std::unique_ptr<FunctionAST>>& Parser::AST() {
    return expr_;
}

std::unique_ptr<ExprAST> Parser::LogError(const std::string& errmsg) {
    std::cout << errmsg << std::endl;
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::LogErrorP(const std::string& errmsg) {
    std::cout << errmsg << std::endl;
    return nullptr;
}

Token& Parser::getNextToken() {
    return cur_token_ = lexer_.nextToken();
}
