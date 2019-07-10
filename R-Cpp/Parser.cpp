#include "Parser.h"
#include <stack>
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

std::unique_ptr<ExprAST> Parser::ParseStatement() {
    if (cur_token_.type == TokenType::Return) {
        return ParseReturnExpr();
    }
    if (cur_token_.type == TokenType::If) {
        return ParseIfExpr();
    }
    if (cur_token_.type == TokenType::For) {
        return ParseForExpr();
    }
    return ParseExpression();
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
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
    //if(cur_token_.type!=TokenType::lBrace) {
    //    return LogError("Expected {");
    //}
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

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;
    std::stack<OperatorType> ops;
    std::stack<std::unique_ptr<ExprAST>> expr;
    ops.push(OperatorType::None);
    expr.push(std::move(LHS));
    while (true)
    {
        if (cur_token_.type == TokenType::Semicolon||cur_token_.type==TokenType::rParenthesis) 
            break;
        auto op = getNextBinOperator();
        if(getBinOperatorPrecedence(op)<getBinOperatorPrecedence(ops.top()))
        {
            while (getBinOperatorPrecedence(op) < getBinOperatorPrecedence(ops.top()))
            {
                if (expr.empty()) return LogError("Incomplete expression.");
                auto r = std::move(expr.top());
                expr.pop();
                if (expr.empty()) return LogError("Incomplete expression.");
                auto l = std::move(expr.top());
                expr.pop();
                auto o = ops.top();
                ops.pop();
                auto e = std::make_unique<BinaryExprAST>(o, std::move(l), std::move(r));
                expr.push(std::move(e));
            }
        }
        ops.push(op);
        expr.push(ParsePrimary());
    }
    while (ops.top()!=OperatorType::None)
    {
        if (expr.empty()) return LogError("Incomplete expression.");
        auto r = std::move(expr.top());
        expr.pop();
        if (expr.empty()) return LogError("Incomplete expression.");
        auto l = std::move(expr.top());
        expr.pop();
        auto o = ops.top();
        ops.pop();
        auto e = std::make_unique<BinaryExprAST>(o, std::move(l), std::move(r));
        expr.push(std::move(e));
    }
    return std::move(expr.top());
}

OperatorType Parser::getNextBinOperator()
{
    auto ret = [&](OperatorType o) {
        getNextToken();
        return o;
    };
    TokenType first = cur_token_.type;
    getNextToken();
    if(first==TokenType::Colon)
    {
        if (cur_token_.type == TokenType::Colon)   // ::
            return ret(OperatorType::ScopeResolution);
        else
            return OperatorType::None;
    }
    if(first==TokenType::Minus)
    {
        if (cur_token_.type == TokenType::rAngle)  // ->
            return ret(OperatorType::MemberAccessA);
        if (cur_token_.type == TokenType::Equal)   // -=
            return ret(OperatorType::MinComAssign);
        return OperatorType::Subtraction;
    }
    if(first==TokenType::lAngle)
    {
        if (cur_token_.type == TokenType::lAngle) {  // <<
            getNextToken();
            if(cur_token_.type==TokenType::Equal)    // <<=
                return ret(OperatorType::LshComAssign);
            return OperatorType::LeftShift;
        }
        if (cur_token_.type == TokenType::Equal)      // <=
            return ret(OperatorType::LessEqual);
        return OperatorType::Less;
    }
    if(first==TokenType::rAngle)
    {
        if(cur_token_.type==TokenType::rAngle)         // >>
        {
            getNextToken();
            if (cur_token_.type == TokenType::Equal)   // >>=
                return ret(OperatorType::RshComAssign);
            return OperatorType::RightShift;
        }
        if (cur_token_.type == TokenType::Equal)       // >=
            return ret(OperatorType::GreaterEqual);
        return OperatorType::Greater;
    }
    if(first==TokenType::Exclam)
    {
        if (cur_token_.type == TokenType::Equal)       // !=
            return ret(OperatorType::NotEqual);
        return OperatorType::None;
    }
    if(first==TokenType::Multiply)
    {
        if (cur_token_.type == TokenType::Equal)        // *=
            return ret(OperatorType::MulComAssign);
        return OperatorType::Multiplication;
    }
    if(first==TokenType::Divide)
    {
        if (cur_token_.type == TokenType::Equal)
            return ret(OperatorType::DivComAssign);
        return OperatorType::Division;
    }
    if(first==TokenType::Plus)
    {
        if (cur_token_.type == TokenType::Equal)
            return ret(OperatorType::SumComAssign);
        return OperatorType::Addition;
    }
    if(first==TokenType::Percent)
    {
        if (cur_token_.type == TokenType::Equal)
            return ret(OperatorType::RemComAssign);
        return OperatorType::Remainder;
    }
    if(first==TokenType::And)
    {
        if (cur_token_.type == TokenType::And)
            return ret(OperatorType::LogicalAND);
        if (cur_token_.type == TokenType::Equal)
            return ret(OperatorType::ANDComAssign);
        return ret(OperatorType::BitwiseAND);
    }
    if(first==TokenType::Or)
    {
        if(cur_token_.type==TokenType::Or)
            return ret(OperatorType::LogicalOR);
        if (cur_token_.type == TokenType::Equal)
            return ret(OperatorType::ORComAssign);
        return OperatorType::BitwiseOR;
    }
    if(first==TokenType::Xor)
    {
        if (cur_token_.type == TokenType::Xor)
            return ret(OperatorType::XORComAssign);
        return OperatorType::None;
    }
    if(first==TokenType::Equal) {
        if (cur_token_.type == TokenType::Equal)
            return ret(OperatorType::Equal);
        return OperatorType::Assignment;
    }
    return TokenToBinOperator(first);
}


//std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
//    while (true) {
//        int TokPrec = GetTokPrecedence();
//        if (TokPrec < ExprPrec)
//            return LHS;
//        auto BinOp = getNextBinOperator();
//        auto RHS = ParsePrimary();
//        if (!RHS)
//            return nullptr;
//        int NextPrec = GetTokPrecedence();
//        if (TokPrec < NextPrec) {
//            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
//            if (!RHS)
//                return nullptr;
//        }
//        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
//                                              std::move(RHS));
//    }
//}

std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
    getNextToken();  // eat if
    auto cond = ParseParenExpr();
    if (!cond) return nullptr;
 //   if (cur_token_.type != TokenType::lBrace)
 //       return LogError("Expected {.");
    auto then = ParseBlock();
    if(cur_token_.type==TokenType::Else) {
        getNextToken();
        if (cur_token_.type == TokenType::If) {
            auto elseif = ParseBlock();
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
    std::vector<std::unique_ptr<ExprAST>> body;
    if(cur_token_.type!=TokenType::lBrace)
    {
        auto E = ParseStatement();
        if (E != nullptr) body.emplace_back(std::move(E));
        if (cur_token_.type == TokenType::Semicolon)
        {
            getNextToken();  //eat ;
        }
        
        return std::make_unique<BlockExprAST>(std::move(body));
    }
    getNextToken(); //eat {

    while (cur_token_.type != TokenType::rBrace && cur_token_.type != TokenType::Eof) {
        auto E = ParseStatement();
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
