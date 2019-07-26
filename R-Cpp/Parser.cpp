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
        auto e = ParseIdentifierExpr();
        while (isPostOperator()) 
        {
            e = ParsePostOperator(std::move(e));
        }
        return e;
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
    auto o = getNextUnaryOperator();
    if(o!=OperatorType::None)
    {
        auto e = ParsePrimary();
        return std::make_unique<UnaryExprAST>(std::move(e),o,e->getType());
    }
    error("Unexpected token.");
    return nullptr;
}

bool Parser::isPostOperator()
{
    if (cur_token_.type == TokenType::lSquare) {
        return true;
    } else if (cur_token_.type == TokenType::lParenthesis) {
        return true;
    } else if (cur_token_.type == TokenType::Point) {
        return true;
    } else if (cur_token_.type == TokenType::Plus) {
        return  lexer_.nextChar() == '+';
    } else if (cur_token_.type == TokenType::Minus) {
        return  lexer_.nextChar() == '-'||lexer_.nextChar()=='>';
    } else if(cur_token_.type==TokenType::Colon)
    {
        return true;
    }
    return false;
}

std::unique_ptr<ExprAST> Parser::ParsePostOperator(std::unique_ptr<ExprAST> e)
{
    auto type = e->getType();
    if (cur_token_.type == TokenType::lSquare) {
        if (type.typeName != "Arr") {
            error("No suitable [] operator for " + type.typeName + ".");
            return nullptr;
        }
        e = std::make_unique<UnaryExprAST>(std::move(e),
            OperatorType::Subscript, ParseSquareExprList(), type.templateArgs[0]);
    } else if (cur_token_.type == TokenType::lParenthesis) {

        //  e = std::make_unique<UnaryExprAST>(std::move(e), 
        //      OperatorType::FunctionCall, ParseParenExprList());
    } else if (cur_token_.type == TokenType::Point) {
        getNextToken();
        e = ParseMemberAccess(std::move(e),
            OperatorType::MemberAccessP);
    } else if (cur_token_.type == TokenType::Plus) {
        assert(getNextUnaryOperator() == OperatorType::PreIncrement);
        e = std::make_unique<UnaryExprAST>(std::move(e), OperatorType::PostIncrement, e->getType());
    } else if (cur_token_.type == TokenType::Minus) {
        if(lexer_.nextChar()=='>')
        {
            assert(e->getType().typeName == "__ptr");
            e = std::make_unique<UnaryExprAST>(std::move(e), OperatorType::Dereference, e->getType().templateArgs[0]);
            assert(getNextBinOperator() == OperatorType::MemberAccessA);
            e = ParseMemberAccess(std::move(e), OperatorType::MemberAccessP);
        }else
        {
            assert(getNextUnaryOperator() == OperatorType::PreDecrement);
            e = std::make_unique<UnaryExprAST>(std::move(e), OperatorType::PostDecrement, e->getType());
        }

    } else if(cur_token_.type==TokenType::Colon)
    {
        assert(getNextBinOperator() == OperatorType::ScopeResolution);
        auto p = dynamic_cast<NamespaceExprAST*>(e.get());
        if(p==nullptr)
        {
            error("Left side of :: is not a namespace identifier.");
            return nullptr;
        }
        cur_namespace_.push_back(p->getName());
        e = ParsePrimary();
        cur_namespace_.clear();
    }
    return e;
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    // ( expression )
    getNextToken(); //eat (
    auto expr = ParseExpression();
    if (!expr) return nullptr;
    if (cur_token_.type != TokenType::rParenthesis) {
        error("Expected ).");
        return nullptr;
    }
    getNextToken(); //eat )
    return expr;
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
    auto idname = cur_token_.content;
    getNextToken();
    // is defining a identifier
 //   if (cur_token_.type_llvm == TokenType::Identifier||cur_token_.type_llvm==TokenType::lAngle) {
    if(symbol_->hasType(idname)&&(cur_token_.type == TokenType::Identifier || cur_token_.type == TokenType::lAngle)){
        return ParseVaribleDefinition(idname);
    }
    if(cur_token_.type==TokenType::Colon)
    {
        return std::make_unique<NamespaceExprAST>(idname);
    }
    // is only a identifier
    if (cur_token_.type != TokenType::lParenthesis) {
        auto var = symbol_->getValue(idname);
        if(var.name=="")
        {
            error("Unknown identifier.");
            return nullptr;
        }
        return std::make_unique<VariableExprAST>(idname,var.type);;
    }
    // is calling a function
    getNextToken();      // eat (
    std::vector<std::unique_ptr<ExprAST>> args;
    if (cur_token_.type != TokenType::rParenthesis) {
        while (true) {
            if (auto arg = ParseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }
            if (cur_token_.type == TokenType::rParenthesis) break;
            if (cur_token_.type != TokenType::Comma)
            {
                error("Expected , or ).");
                return nullptr;
            }
            getNextToken();
        }
    }
    getNextToken();
    auto fnList = symbol_->getRawFunction(idname,cur_namespace_);
    Function target;
    for(auto&f:*fnList)
    {
        bool flag = true;
        if(f.args.size()==args.size())
        {
            size_t i = 0;
            while(i<f.args.size())
            {
                if(f.args[i].type!=args[i]->getType())
                {
                    flag = false;
                    break;
                }
                ++i;
            }
            if (flag) target = f;
        }
    }
    if(target.name=="")
    {
        error("No suitable function.");
        return nullptr;
    }
    return std::make_unique<CallExprAST>(Function::mangle(target), std::move(args),target.returnType);
}

std::unique_ptr<ExprAST> Parser::ParseForExpr() {
    SymbolTable::ScopeGuard sg(*symbol_);
    getNextToken(); //eat for
    if(cur_token_.type!=TokenType::lParenthesis) {
        error("Expected ( after a for loop.");
        return nullptr;
    }
    getNextToken(); //eat (
    auto start = ParseExpression();
    if(cur_token_.type!=TokenType::Semicolon) {
        error("Expected ; to split for expression.");
        return nullptr;
    }
    getNextToken(); //eat ;
    auto cond = ParseExpression();
    if (cur_token_.type != TokenType::Semicolon) {
        error("Expected ; to split for expression.");
        return nullptr;
    }
    getNextToken(); //eat ;
    auto end = ParseExpression();
    if (cur_token_.type != TokenType::rParenthesis) {
        error("Expected ) to end for condition.");
        return nullptr;
    }
    getNextToken(); //eat )
    auto body = ParseBlock();
    return std::make_unique<ForExprAST>(std::move(start), std::move(cond), std::move(end), std::move(body));
}

std::unique_ptr<ExprAST> Parser::ParseVaribleDefinition(const std::string& type_name) {
    std::vector<VarType> template_args;
    if(cur_token_.type==TokenType::lAngle)
    {
        template_args = ParseAngleExprList();
    }
    auto varname = cur_token_.content;
    auto type = VarType(type_name);
    type.templateArgs = template_args;
    symbol_->setValue(varname, Variable(varname, type));
    getNextToken();
    auto expr = std::make_unique<VariableDefAST>(type, varname);
    if (cur_token_.type == TokenType::Equal) {
        // definition with initiate value
        getNextToken();   // eat =
        auto E = ParseExpression();
        expr->setInitValue(std::move(E));
    }
    else if (cur_token_.type != TokenType::Semicolon) {
        error("Expected initiate value or ;.");
        return nullptr;
    }
    return expr;
}

std::unique_ptr<ExprAST> Parser::ParseReturnExpr() {
    getNextToken();
    auto retval = ParseExpression();
    if (retval != nullptr) return std::make_unique<ReturnAST>(std::move(retval));
    getNextToken();
    error("Invalid return value.");
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::MergeExpr(std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS, OperatorType Op)
{
    if (Op != OperatorType::MemberAccessP) {
        return std::make_unique<BinaryExprAST>(Op, std::move(LHS),
            std::move(RHS), builtinOperatorReturnType(LHS->getTypeName(), RHS->getTypeName(), Op));
    }
    error("Unexpected operator.");
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;
    std::stack<OperatorType> ops;
    ops.push(OperatorType::None);
    std::stack<std::unique_ptr<ExprAST>> expr;
    expr.push(std::move(LHS));
    while (true)
    {
        OperatorType op;
        if (cur_token_.type == TokenType::Semicolon||cur_token_.type==TokenType::rParenthesis
            ||cur_token_.type==TokenType::rSquare)
        {
            if (ops.top() == OperatorType::None) break;
            op = OperatorType::None;
        }else
            op = getNextBinOperator();
        while(getBinOperatorPrecedence(op)<getBinOperatorPrecedence(ops.top()))
        {
            std::stack<OperatorType> curLevelOps;
            std::stack<std::unique_ptr<ExprAST>> curLevelExprs;
            curLevelExprs.push(std::move(expr.top()));
            expr.pop();
            auto curPrecedence = getBinOperatorPrecedence(ops.top());
            while (curPrecedence== getBinOperatorPrecedence(ops.top()))
            {
                curLevelOps.push(ops.top());
                ops.pop();
                curLevelExprs.push(std::move(expr.top()));
                expr.pop();
            }
            while (!curLevelOps.empty())
            {
                auto o = curLevelOps.top();
                curLevelOps.pop();
                auto l = std::move(curLevelExprs.top());
                curLevelExprs.pop();
                auto r = std::move(curLevelExprs.top());
                curLevelExprs.pop();
                curLevelExprs.push(MergeExpr(std::move(l), std::move(r), o));
            }
            expr.push(std::move(curLevelExprs.top()));
            curLevelExprs.pop();
            assert(curLevelExprs.empty()==true);
        }
        if(op!=OperatorType::None)
        {
            ops.push(op);
            expr.push(ParsePrimary());
        }
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

OperatorType Parser::getNextUnaryOperator()
{
    auto ret = [&](OperatorType o) {
        getNextToken();
        return o;
    };
    TokenType first = cur_token_.type;
    getNextToken();
    if(first==TokenType::Plus)
    {
        if (cur_token_.type == TokenType::Plus)
            return ret(OperatorType::PreIncrement);
        return OperatorType::Promotion;
    }
    if(first==TokenType::Minus)
    {
        if (cur_token_.type == TokenType::Minus)
            return ret(OperatorType::PreDecrement);
        return OperatorType::Negation;
    }
    if(first==TokenType::Exclam)
    {
        return OperatorType::LogicalNOT;
    }
    if(first==TokenType::Tilde)
    {
        return OperatorType::BitwiseNOT;
    }
    if(first==TokenType::Multiply)
    {
        return OperatorType::Dereference;
    }
    return OperatorType::None;
}

std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
    getNextToken();  // eat if
    auto cond = ParseParenExpr();
    if (!cond) return nullptr;
 //   if (cur_token_.type_llvm != TokenType::lBrace)
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
    {
        error("Expected function name in prototype");
        return nullptr;
    }
    std::string FnName = cur_token_.content;
    getNextToken();
    if (cur_token_.type != TokenType::lParenthesis) {
        error("Expected '(' in prototype");
        return nullptr;
    }
    // Read the list of argument names.
    std::vector<Variable> ArgNames;
    // std::vector<std::pair<std::string, std::string>> ArgNames;
    while (getNextToken().type == TokenType::Identifier) {
        auto type_name = cur_token_.content;
        getNextToken();
        ArgNames.emplace_back(cur_token_.content,type_name);
        getNextToken();
        if (cur_token_.type == TokenType::rParenthesis) break;
        if (cur_token_.type != TokenType::Comma)
        {
            error("Expected , to split arguments.");
            return nullptr;
        }
    }

    if (cur_token_.type != TokenType::rParenthesis) {
        error("Expected ')' in prototype");
        return nullptr;
    }

    getNextToken(); // eat ')'.
    VarType retType;
    if(cur_token_.type==TokenType::Minus)
    {
        auto op = getNextBinOperator();
        if(op!=OperatorType::MemberAccessA)
        {
            error("Unexpected symbol.");
            return nullptr;
        }
        if(cur_token_.type!=TokenType::Identifier||!symbol_->hasType(cur_token_.content))
        {
            error("Unknown type.");
            return nullptr;
        }
        retType = ParseType();
        if(!symbol_->hasType(retType))
        {
            error("Unknown return type " + retType.typeName + ".");
        }
    }else if(cur_token_.type!=TokenType::lBrace)
    {
        error("Return value must be pointed out explicitly when declaring a function prototype.");
        return nullptr;
    }
    auto p = std::make_unique<PrototypeAST>(Function(FnName, ArgNames, retType, isExternal));
    if (cur_class_.typeName != "") 
        p->setClassType(cur_class_);
    return p;
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
        if (!E) return nullptr;
        body.emplace_back(std::move(E));
        if(cur_token_.type==TokenType::Semicolon)
        getNextToken(); // eat ;
    }
    if (cur_token_.type != TokenType::rBrace) {
        error("Expected }.");
        return nullptr;
    }
    getNextToken(); //eat }
    return std::make_unique<BlockExprAST>(std::move(body));
}

std::unique_ptr<FunctionAST> Parser::ParseFunction() {
    getNextToken(); // eat fn.
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;
    symbol_->addRawFunction(Proto->getFunction());
    auto f = Proto->getFunction();
    auto name = ::Function::mangle(f);
    proto_.emplace_back(std::move(Proto));
    SymbolTable::ScopeGuard sg(*symbol_);
    for(auto it:f.args)
    {
        symbol_->setValue(it.name, Variable(it.name, it.type));
    }
    if(cur_token_.type==TokenType::Semicolon)
    {
        getNextToken();
        return std::make_unique<FunctionAST>(name, nullptr);
    }
    if (cur_token_.type != TokenType::lBrace) {
        error("Expected { or ;.");
        return nullptr;
    }
    auto body = ParseBlock();
    if (!body) {
        return nullptr;
    }
    return std::make_unique<FunctionAST>(name, std::move(body));
}

void Parser::HandleDefinition() {
    auto func = ParseFunction();
    if (func != nullptr) {
        fprintf(stderr, "Parsed a function definition.\n");
        expr_.emplace_back(std::move(func));
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

std::unique_ptr<ClassAST> Parser::ParseClass()
{
    getNextToken();  // eat class
    if (cur_token_.type != TokenType::Identifier) {
        error("Expected class name.");
        return nullptr;
    }
    auto className = cur_token_.content;
    Class c(className);
    getNextToken();
    if (cur_token_.type != TokenType::lBrace) {
        error("Expect class body.");
        return nullptr;
    }

    getNextToken(); // eat ;
    {
        SymbolTable::ScopeGuard guard(*symbol_);
        SymbolTable::NamespaceGuard ns(*symbol_,className);
        auto tmp = cur_class_;
        cur_class_ = c.type;
        while (cur_token_.type != TokenType::rBrace) {
            if (cur_token_.type == TokenType::Identifier) {
                auto type = cur_token_.content;
                getNextToken();
                auto res = ParseVaribleDefinition(type);
                auto var = dynamic_cast<VariableDefAST*>(res.get());
                if (!var) {
                    error("Unknown error.");
                    return nullptr;
                }
                symbol_->getClass(c.type);
                c.memberVariables.emplace_back(var->getVarName(), var->getVarType());
            } else if (cur_token_.type == TokenType::Function) {
                auto fn = ParseFunction();
                Function f(proto_.back()->getFunction());
                c.memberFunctions.push_back(f);
                expr_.push_back(std::move(fn));
                //symbol_->addFunction(f.name, f);
            }
            if (cur_token_.type == TokenType::Semicolon) getNextToken();
        }
        symbol_->addRawFunction(generateFunction_new(c.type));
        cur_class_ = tmp;
    }
    symbol_->addClass(className, c);
    return std::make_unique<ClassAST>(c);
}

void Parser::HandleClass()
{
    auto c = ParseClass();
    if(c)
    {
        fprintf(stderr, "Parsed a class.\n");
        class_.push_back(std::move(c));
    }else
    {
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
        case TokenType::Class:
            HandleClass();
            break;
        case TokenType::External:
            ParseExternal();
            break;
        case TokenType::Internal:
            ParseInternal();
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

Token& Parser::getNextToken() {
    return cur_token_ = lexer_.nextToken();
}

void Parser::error(const std::string& errmsg)
{
    std::cout << lexer_.getLineNo() << "." << lexer_.getCharNo() << ":\t";
    std::cout << errmsg << std::endl;
}

std::vector<std::unique_ptr<ClassAST>>& Parser::Classes()
{
    return class_;
}

std::vector<std::unique_ptr<PrototypeAST>>& Parser::Prototypes()
{
    return proto_;
}

std::vector<std::unique_ptr<ExprAST>> Parser::ParseExprList(TokenType endToken)
{
    std::vector<std::unique_ptr<ExprAST>> exprs;
    while (cur_token_.type!=endToken)
    {
        exprs.push_back(ParseExpression());
        if (cur_token_.type == endToken) break;
        if(cur_token_.type!=TokenType::Comma)
        {
            error("Expect , to split expressions.");
            return std::vector<std::unique_ptr<ExprAST>>();
        }
        getNextToken();  // eat ,
    }
    return exprs;
}

std::vector<std::unique_ptr<ExprAST>> Parser::ParseParenExprList()
{
    getNextToken(); // eat (
    auto exprs = ParseExprList(TokenType::rParenthesis);
    getNextToken();  // eat )
    return exprs;
}

std::vector<std::unique_ptr<ExprAST>> Parser::ParseSquareExprList()
{
    getNextToken(); // eat [
    auto exprs = ParseExprList(TokenType::rSquare);
    getNextToken();  // eat ]
    return exprs;
}

std::vector<VarType> Parser::ParseAngleExprList()
{
    getNextToken(); // eat <
    //auto exprs = ParseExprList(TokenType::rAngle);
    std::vector<VarType> exprs;
    while (cur_token_.type != TokenType::rAngle) {
        exprs.push_back(ParseType());
        if(cur_token_.type==TokenType::Comma)
        {
            getNextToken();
        }else if(cur_token_.type!=TokenType::rAngle)
        {
            error("Expect , to split typename or > to end list.");
            return std::vector<VarType>();
        }
    }
    getNextToken();  // eat >
    return exprs;
}

VarType Parser::ParseType()
{
    /*bool isconst = false;
    if(cur_token_.content=="const")
    {
        isconst = true;
    }*/
    auto type = VarType(cur_token_.content);
    getNextToken();
    if(cur_token_.type==TokenType::lAngle)
    {
        type.templateArgs = ParseAngleExprList();
    }
    return VarType(type);
}

std::unique_ptr<ExprAST> Parser::ParseMemberAccess(std::unique_ptr<ExprAST> lhs,OperatorType Op)
{
    auto type = lhs->getType();
    auto name = cur_token_.content;
    getNextToken();
    if(cur_token_.type==TokenType::lParenthesis)
    {
        Function F;
        auto args = ParseParenExprList();
        for (auto& f : symbol_->getClass(type).memberFunctions) {
            if (f.name == name&&f.args.size()==args.size()) {
                size_t i = 0;
                bool flag = true;
                while (i<args.size())
                {
                    if(F.args[i].type!=args[i]->getType())
                    {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    F = f; break;
                }
            }
        }
        if(F.name=="")
        {
            error("No suitable function for " + type.typeName + ".");
            return nullptr;
        }
        auto call = std::make_unique<CallExprAST>(Function::mangle(F), std::move(args), F.returnType);
        call->setThis(std::move(lhs));
        return call;
    }else
    {
        VarType retType;
        for(auto& m:symbol_->getClass(type).memberVariables)
        {
            if(m.name==name)
            {
                retType = m.type;
            }
        }
        if(retType.typeName=="")
        {
            error("No '" + name + "' in class " + type.typeName + ".");
            return nullptr;
        }
        return std::make_unique<MemberAccessAST>(std::move(lhs), name, Op, retType);
    }
}

Function Parser::generateFunction_new(const VarType& t)
{
    Function f;
    f.name = "new";
    f.classType = t;
    VarType retType("__ptr");
    retType.templateArgs.push_back(t);
    f.returnType = retType;
    return f;
}

std::shared_ptr<SymbolTable> Parser::symbolTable()
{
    return symbol_;
}

void Parser::ParseExternal()
{
    isExternal = true;
    getNextToken();
    if (cur_token_.type != TokenType::Colon)
        error("Expected : after external.");
    getNextToken();
}
void Parser::ParseInternal() {
    isExternal = false;
    getNextToken();
    if (cur_token_.type != TokenType::Colon)
        error("Expected : after internal.");
    getNextToken();
}
