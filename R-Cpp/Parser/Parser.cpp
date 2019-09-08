#include "Parser.h"
#include <stack>
#include <iostream>

using namespace Parse;

std::unique_ptr<Stmt> Parser::ParseIntegerExpr() {
    auto res = std::make_unique<IntegerStmt>(std::stoi(cur_token_.content));
    getNextToken();
    return res;
}

std::unique_ptr<Stmt> Parser::ParseFloatExpr() {
    auto res = std::make_unique<FloatStmt>(std::stof(cur_token_.content));
    getNextToken();
    return res;
}

std::unique_ptr<Stmt> Parse::Parser::ParseStatement() {
    if (cur_token_.type == TokenType::Return) {
        return ParseReturnExpr();
    }
    if (cur_token_.type == TokenType::If) {
        return ParseIfExpr();
    }
    if (cur_token_.type == TokenType::For) {
        return ParseForExpr();
    }
    // TODO: Using statement
    //if(cur_token_.type==TokenType::Using)
    //{
    //    ParseUsing();
    //    return std::make_unique<NonExprAST>();
    //}
    return ParseExpression();
}

std::unique_ptr<Stmt> Parse::Parser::ParsePrimary() {
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
    auto op = getNextUnaryOperator();
    if(op!=OperatorType::None)
    {
        auto expr = ParsePrimary();
        return std::make_unique<UnaryOperatorStmt>(std::move(expr),op);
    }
    error("Unexpected token.");
    return nullptr;
}

bool Parse::Parser::isPostOperator()
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

std::unique_ptr<Stmt> Parse::Parser::ParsePostOperator(std::unique_ptr<Stmt> e)
{
    //auto type = e->getType();
    if (cur_token_.type == TokenType::lSquare) {
        //if (type.typeName != "__arr") {
        //    error("No suitable [] operator for " + type.typeName + ".");
        //    return nullptr;
        //}
        e = std::make_unique<UnaryOperatorStmt>(std::move(e),
            OperatorType::Subscript, ParseSquareExprList());
    } else if (cur_token_.type == TokenType::lParenthesis) {
        e = std::make_unique<UnaryOperatorStmt>(std::move(e),
              OperatorType::FunctionCall, ParseParenExprList());
    } else if (cur_token_.type == TokenType::Point) {
        getNextToken();
        e= std::make_unique<BinaryOperatorStmt>(std::move(e), ParseExpression(),
            OperatorType::MemberAccessP);
        //e = ParseMemberAccess(std::move(e), OperatorType::MemberAccessP);
    } else if (cur_token_.type == TokenType::Plus) {
        assert(getNextUnaryOperator() == OperatorType::PreIncrement);
        e = std::make_unique<UnaryOperatorStmt>(std::move(e), OperatorType::PostIncrement);
    } else if (cur_token_.type == TokenType::Minus) {
        if(lexer_.nextChar()=='>')
        {
            //assert(e->getType().typeName == "__ptr");
            assert(getNextBinOperator() == OperatorType::MemberAccessA);
            e = std::make_unique<BinaryOperatorStmt>(std::move(e),ParseExpression(), OperatorType::MemberAccessA);
            //e = ParseMemberAccess(std::move(e), OperatorType::MemberAccessP);
        }else
        {
            assert(getNextUnaryOperator() == OperatorType::PreDecrement);
            e = std::make_unique<UnaryOperatorStmt>(std::move(e), OperatorType::PostDecrement);
        }

    }
    // TODO: Namspace
    /* else if(cur_token_.type==TokenType::Colon)
    {
        assert(getNextBinOperator() == OperatorType::ScopeResolution);
        e=
        auto p = dynamic_cast<NamespaceExprAST*>(e.get());
        if(p==nullptr)
        {
            error("Left side of :: is not a namespace identifier.");
            return nullptr;
        }
        cur_namespace_.push_back(p->getName());
        e = ParsePrimary();
        cur_namespace_.clear();
    }*/
    return e;
}

std::unique_ptr<Stmt> Parse::Parser::ParseParenExpr() {
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

std::unique_ptr<Stmt> Parse::Parser::ParseIdentifierExpr() {
    auto idname = cur_token_.content;
    getNextToken();
    // is defining a identifier
 //   if (cur_token_.type_llvm == TokenType::Identifier||cur_token_.type_llvm==TokenType::lAngle) {
    if(symbol_->hasType(idname)&&(cur_token_.type == TokenType::Identifier || cur_token_.type == TokenType::lAngle || cur_token_.type==TokenType::lParenthesis)){
        return ParseVariableDefinition(idname);
    }
    // TODO: Namspace
    //if(cur_token_.type==TokenType::Colon)
    //{
    //    return std::make_unique<NamespaceExprAST>(idname);
    //}
    // is only a identifier
    if (cur_token_.type != TokenType::lParenthesis) {
        //auto var = symbol_->getValue(idname);
        //if(var.name=="")
        //{
        //    error("Unknown identifier.");
        //    return nullptr;
        //}
        return std::make_unique<VariableStmt>(idname);;
    }
    // TODO: is calling a function
    /*assert(cur_token_.type == TokenType::lParenthesis);
    auto args = ParseParenExprList();
    auto fnList = symbol_->getFunction(idname,cur_namespace_);
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
    return std::make_unique<CallExprAST>(Function::mangle(target), std::move(args),target.returnType);*/
}

std::unique_ptr<Stmt> Parse::Parser::ParseForExpr() {
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
    //sg.setBlock(body.get());
    return std::make_unique<ForStmt>(std::move(start), std::move(cond), std::move(end), std::move(body));
}

std::unique_ptr<Stmt> Parse::Parser::ParseVariableDefinition(const std::string& type_name) {
    std::vector<VarType> template_args;
    if(cur_token_.type==TokenType::lAngle)
    {
        template_args = ParseAngleExprList();
    }
    auto type = VarType(type_name);
    type.templateArgs = template_args;
    type.typeName = symbol_->getMangledClassName(type);
    if(cur_token_.type==TokenType::Identifier)
    {   // like 'int p=0'
        auto varname = cur_token_.content;
        //if (type_name == "__arr") {
        //    if (template_args.size() != 2) {
        //        error("Invalid count of arguments for Arr.");
        //        return nullptr;
        //    }

        //} else if (type_name == "__ptr") {
        //    if (template_args.size() != 1) {
        //        error("Invalid count of arguments for __ptr.");
        //        return nullptr;
        //    }
        //} else if (type.typeName == "") {
        //    error("No type named " + type_name + ".");
        //    return nullptr;
        //}
        //symbol_->setValue(varname, Variable(varname, type));
        getNextToken();
        auto expr = std::make_unique<VariableDefStmt>(type, varname);
        //if (type_name == "__arr" || type_name == "__ptr") {
        //    std::vector<std::string> tempargs;
        //    for (auto& v : type.templateArgs) {
        //        tempargs.push_back(::VarType::mangle(v));
        //    }
        //    expr->setTemplateArgs(tempargs);
        //}
        if (cur_token_.type == TokenType::Equal) {
            // definition with initiate value
            getNextToken();   // eat =
            auto E = ParseExpression();
            expr->setInitValue(std::move(E));
        } else if (cur_token_.type != TokenType::Semicolon) {
            error("Expected initiate value or ;.");
            return nullptr;
        }
        return expr;
    }else
    {   // like 'Var(10)'
        assert(cur_token_.type == TokenType::lParenthesis);
        auto args = ParseParenExprList();
        /*auto clas = symbolTable()->getClass(::VarType::mangle(type));
        if(clas.type.typeName=="")
        {
            error("No class named " + type.typeName);
        }
        Function  target;
        for(auto& f:clas.constructors)
        {
            if(f.args.size()==args.size())
            {
                bool flag = true;
                for (size_t i = 0; i < f.args.size(); ++i) {
                    if (f.args[i].type != args[i]->getType()) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    target = f;
                    break;
                }
            }
        }
        if(target.name=="")
        {
            error("No suitable constructor.");
            return nullptr;
        }
        std::vector<std::unique_ptr<Stmt>> exprlist;
        std::string varname = "__" + std::to_string(nameless_var_count_++)+"tmp_";*/
        //symbolTable()->addNamelessVariable(Variable(varname, type));
        return std::make_unique<NamelessVariableStmt>(type, std::move(args));
    }
}

std::unique_ptr<Stmt> Parse::Parser::ParseReturnExpr() {
    getNextToken();
    std::unique_ptr<Stmt> retval=nullptr;
    if(cur_token_.type!=TokenType::Semicolon)
    {
        retval = ParseExpression();
        if(!retval)
        {
            error("Invalid return value.");
            return nullptr;
        }
    }
    /*std::vector<std::unique_ptr<Stmt>> destructor;
    symbolTable()->callNamelessVarDestructor(destructor);
    auto des2 = symbolTable()->callDestructor();
    destructor.insert(destructor.end(), std::make_move_iterator(des2.begin()), std::make_move_iterator(des2.end()));*/
    return std::make_unique<ReturnStmt>(std::move(retval));
}

std::unique_ptr<Stmt> Parse::Parser::MergeExpr(std::unique_ptr<Stmt> LHS, std::unique_ptr<Stmt> RHS, OperatorType Op)
{
    if (Op != OperatorType::MemberAccessP) {
        return std::make_unique<BinaryOperatorStmt>(std::move(LHS),
            std::move(RHS), Op);
    }
    error("Unexpected operator.");
    return nullptr;
}

std::unique_ptr<Stmt> Parse::Parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;
    std::stack<OperatorType> ops;
    ops.push(OperatorType::None);
    std::stack<std::unique_ptr<Stmt>> expr;
    expr.push(std::move(LHS));
    while (true)
    {
        OperatorType op;
        if (cur_token_.type == TokenType::Semicolon||cur_token_.type==TokenType::rParenthesis
            ||cur_token_.type==TokenType::rSquare||cur_token_.type==TokenType::Comma)
        {
            if (ops.top() == OperatorType::None) break;
            op = OperatorType::None;
        }else
            op = getNextBinOperator();
        while(getBinOperatorPrecedence(op)<getBinOperatorPrecedence(ops.top()))
        {
            std::stack<OperatorType> curLevelOps;
            std::stack<std::unique_ptr<Stmt>> curLevelExprs;
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

OperatorType Parse::Parser::getNextBinOperator()
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

OperatorType Parse::Parser::getNextUnaryOperator()
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

std::unique_ptr<Stmt> Parse::Parser::ParseIfExpr() {
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
            return std::make_unique<IfStmt>(std::move(cond), std::move(then), std::move(elseif));
        }
        auto els = ParseBlock();
        if (!els) return nullptr;
        return std::make_unique<IfStmt>(std::move(cond), std::move(then), std::move(els));
    }
    return std::make_unique<IfStmt>(std::move(cond), std::move(then));
}

::Function Parse::Parser::ParsePrototype() {
    if (cur_token_.type != TokenType::Identifier)
    {
        error("Expected function name in prototype");
        return {};
    }
    std::string FnName = cur_token_.content; 
    getNextToken();
    if (cur_token_.type != TokenType::lParenthesis) {
        error("Expected '(' in prototype");
        return {};
    }
    // Read the list of argument names.
    std::vector<Variable> ArgNames;
    getNextToken();
    while (cur_token_.type != TokenType::rParenthesis) {
        auto type = ParseType();
       // getNextToken();
        ArgNames.emplace_back(Variable(cur_token_.content,type));
        getNextToken();
        if (cur_token_.type == TokenType::rParenthesis) break;
        if (cur_token_.type != TokenType::Comma)
        {
            error("Expected , to split arguments.");
            return {};
        }
        getNextToken();
    }

    if (cur_token_.type != TokenType::rParenthesis) {
        error("Expected ')' in prototype");
        return {};
    }

    getNextToken(); // eat ')'.
    VarType retType;
    if(cur_token_.type==TokenType::Minus)
    {
        auto op = getNextBinOperator();
        if(op!=OperatorType::MemberAccessA)
        {
            error("Unexpected symbol.");
            return {};
        }
        if(cur_token_.type!=TokenType::Identifier||!symbol_->hasType(cur_token_.content))
        {
            error("Unknown type.");
            return {};
        }
        retType = ParseType();
        if(!symbol_->hasType(retType.typeName))
        {
            error("Unknown return type " + retType.typeName + ".");
        }
    }else if(cur_token_.type!=TokenType::lBrace)
    {
        error("Return value must be pointed out explicitly when declaring a function prototype.");
        return {};
    }else
    {
        retType.typeName = "void";
    }
    Function f(FnName, ArgNames, retType, isExternal);
    if (cur_class_.typeName != "")
        f.classType = cur_class_;
    std::vector<std::pair<std::string, std::string>> argList;
    for(auto& v:f.args)
    {
        argList.emplace_back(::VarType::mangle(v.type), v.name);
    }
    //auto p = std::make_unique<PrototypeAST>(::Function::mangle(f),std::move(argList),::VarType::mangle(retType));
    //if (cur_class_.typeName != "")
    //    p->setClassType(::VarType::mangle(cur_class_));
    //proto_.emplace_back(std::move(p));
    //symbol_->addFunction(f);
    return f;
}

std::unique_ptr<CompoundStmt> Parse::Parser::ParseBlock() {
    SymbolTable::ScopeGuard guard(*symbol_);
    std::vector<std::unique_ptr<Stmt>> body;
    bool hasReturn = false;
    if(cur_token_.type!=TokenType::lBrace)
    {   // with only one statement, {} can be omitted
        auto expr = ParseStatement();
        if (expr != nullptr) body.emplace_back(std::move(expr));
        //if (dynamic_cast<ReturnStmt*>(expr.get()) != nullptr) hasReturn = true;
        if (cur_token_.type == TokenType::Semicolon)
        {
            getNextToken();  //eat ;
        }
        //symbol_->callNamelessVarDestructor(body);
    }
    else 
    {   // block with { }
        getNextToken();      //eat {
        while (cur_token_.type != TokenType::rBrace && cur_token_.type != TokenType::Eof) {
            auto E = ParseStatement();
            if (!E) return nullptr;
            //if (dynamic_cast<ReturnAST*>(E.get()) != nullptr) hasReturn = true;
            body.emplace_back(std::move(E));
            if (cur_token_.type == TokenType::Semicolon)
                getNextToken();     // eat ;
            //symbolTable()->callNamelessVarDestructor(body);
        }
        if (cur_token_.type != TokenType::rBrace) {
            error("Expected }.");
            return nullptr;
        }
        getNextToken(); //eat }
    }
    auto block = std::make_unique<CompoundStmt>(std::move(body));
    //if(!hasReturn)
    //    guard.setBlock(block.get());
    return block;
}

::Function Parse::Parser::ParseFunction() {
    getNextToken(); // eat fn.
    auto f = ParsePrototype();
    if (f.name=="") return {};
    auto name = f.name;
    std::unique_ptr<CompoundStmt> body;
    {
        SymbolTable::ScopeGuard sg(*symbol_);
        for (auto it : f.args) {
            symbol_->setValue(it.name, Variable(it.name, it.type));
        }
        if (cur_token_.type == TokenType::Semicolon) {
            getNextToken();
            return f;
        }
        if (cur_token_.type != TokenType::lBrace) {
            error("Expected { or ;.");
            return {};
        }
        body = ParseBlock();
        if (!body) {
            return {};
        }
        //if(!body->hasReturn()&&dynamic_cast<ReturnAST*>(body->instructions().back().get())==nullptr)
        //{
        //    body->instructions().push_back(std::make_unique<ReturnAST>(nullptr, symbolTable()->callDestructor()));
        //    body->setHasReturn();
        //}
      //  sg.setBlock(body.get());
    }
    declartions_.push_back(std::make_unique<FunctionDecl>(f,std::move(body)));
    return f;
}

void Parse::Parser::HandleDefinition() {
    auto func = ParseFunction();
    if (func.name != "") {
        fprintf(stderr, "Parsed a function definition.\n");
        //expr_.emplace_back(std::move(func));
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

std::unique_ptr<ClassDecl> Parse::Parser::ParseClass(VarType classType)
{
    getNextToken();  // eat class
    if (cur_token_.type != TokenType::Identifier) {
        error("Expected class name.");
        return nullptr;
    }
    auto basename = cur_token_.content;
    if(classType.typeName=="")
        classType.typeName = basename;
    Class c(classType);
    getNextToken();
    if (cur_token_.type != TokenType::lBrace) {
        error("Expect class body.");
        return nullptr;
    }
    getNextToken(); // eat ;
    auto classDecl = std::make_unique<ClassDecl>(c.type.typeName);
    //std::unique_ptr<CompoundStmt> desturctorBlock = nullptr;
    {
        SymbolTable::ScopeGuard guard(*symbol_);
        SymbolTable::NamespaceGuard ns(*symbol_,VarType::mangle(c.type));
        auto tmp = cur_class_;
        cur_class_ = c.type;
        while (cur_token_.type != TokenType::rBrace) {
            if (cur_token_.type == TokenType::Identifier) {
                if(cur_token_.content==basename && lexer_.nextChar()=='(')
                {   // parsing constructor
                    getNextToken();
                    std::vector<Variable> ArgNames;
                    getNextToken();
                    while (cur_token_.type != TokenType::rParenthesis) {
                        auto type = ParseType();
                        // getNextToken();
                        ArgNames.emplace_back(Variable(cur_token_.content, type));
                        getNextToken();
                        if (cur_token_.type == TokenType::rParenthesis) break;
                        if (cur_token_.type != TokenType::Comma) {
                            error("Expected , to split arguments.");
                            return {};
                        }
                        getNextToken();
                    }
                    getNextToken();
                    std::unique_ptr<CompoundStmt> b;
                    {
                        SymbolTable::ScopeGuard argGuard(*symbol_);
                        for (auto& v : ArgNames) {
                            symbol_->setValue(v.name, v);
                        }
                        b = ParseBlock();
                        //argGuard.setBlock(b.get());
                    }
                    Function f("__construct", ArgNames, VarType("void"),isExternal);
                    f.classType = c.type;
                    std::vector<std::pair<std::string, std::string>> argList;
                    for(auto&v:ArgNames)
                    {
                        argList.emplace_back(::VarType::mangle(v.type), v.name);
                    }
                    //auto P = std::make_unique<PrototypeAST>(::Function::mangle(f), std::move(argList), "void");
                    //P->setClassType(VarType::mangle(c.type));
                    auto F = std::make_unique<FunctionDecl>(f, std::move(b));
                    //proto_.push_back(std::move(P));
                    //expr_.push_back(std::move(F));
                    classDecl->addMemberFunction(std::move(F));
                    c.constructors.push_back(f);
                }
                else 
                {   // parsing member variable
                    auto type = ParseType();
                    auto name = cur_token_.content;
                    getNextToken();
                    if (name == "") {
                        error("Invalid declaration.");
                        return nullptr;
                    }
                    //symbol_->setValue(name, Variable(name, type));
                    classDecl->addMemberVariable(type, name);
                    //c.memberVariables.emplace_back(name, type);
                }
            } else if (cur_token_.type == TokenType::Function) {
                c.memberFunctions.push_back(ParseFunction());
            } else if (cur_token_.type == TokenType::Tilde) {   // parsing destructor
                if (getNextToken().type != TokenType::Identifier || cur_token_.content != basename) {
                    error("Expected classname to identify destructor.");
                    return nullptr;
                }
                getNextToken();
                auto exprs = ParseParenExprList();
                if(exprs.size()!=0)
                {
                    error("Destructor doesn't take any argument.");
                    return nullptr;
                }
                //desturctorBlock = ParseBlock();
                //auto block = ParseBlock();
                //std::vector<Variable> args;
                //Function f("__destructor", args, VarType("void"), false);
                //f.classType = c.type;
                //std::vector<std::pair<std::string, std::string>> argList;
                //auto P = std::make_unique<PrototypeAST>(::Function::mangle(f), argList, "void");
                //P->setClassType(className);
                //auto F = std::make_unique<FunctionAST>(::Function::mangle(f), std::move(block), className);
                //proto_.push_back(std::move(P));
                //expr_.push_back(std::move(F));
                //c.destructor=f;
            }
            if (cur_token_.type == TokenType::Semicolon) getNextToken();
        }
        //symbol_->addFunction(generateFunction_new(c.type));
        //generateDestructor(c, std::move(desturctorBlock));
        cur_class_ = tmp;
    }
    getNextToken();
    c.type.typeName = VarType::mangle(c.type);
    symbol_->addClass(c.type.typeName, c);
    std::vector<std::pair<std::string, std::string>> members;
    for(auto&v:c.memberVariables)
    {
        members.emplace_back(::VarType::mangle(v.type), v.name);
    }
    return classDecl;
}

void Parse::Parser::HandleClass()
{
    auto c = ParseClass();
    if(c)
    {
        fprintf(stderr, "Parsed a class.\n");
        declartions_.push_back(std::move(c));
    }else
    {
        getNextToken();
    }
}

void Parse::Parser::MainLoop() {
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
        case TokenType::Using:
            ParseUsing();
            break;
        case TokenType::lAngle:
            ParseTemplateClass();
            break;
        default:
            getNextToken();
            //    HandleTopLevelExpression();
            break;
        }
    }
}

Token& Parse::Parser::getNextToken() {
    return cur_token_ = lexer_.nextToken();
}

void Parse::Parser::error(const std::string& errmsg)
{
    std::cout << lexer_.getLineNo() << "." << lexer_.getCharNo() << ":\t";
    std::cout << errmsg << std::endl;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseExprList(TokenType endToken)
{
    std::vector<std::unique_ptr<Stmt>> exprs;
    while (cur_token_.type!=endToken)
    {
        exprs.push_back(ParseExpression());
        if (cur_token_.type == endToken) break;
        if(cur_token_.type!=TokenType::Comma)
        {
            error("Expect , to split expressions.");
            return std::vector<std::unique_ptr<Stmt>>();
        }
        getNextToken();  // eat ,
    }
    return exprs;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseParenExprList()
{
    getNextToken(); // eat (
    auto exprs = ParseExprList(TokenType::rParenthesis);
    getNextToken();  // eat )
    return exprs;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseSquareExprList()
{
    getNextToken(); // eat [
    auto exprs = ParseExprList(TokenType::rSquare);
    getNextToken();  // eat ]
    return exprs;
}

std::vector<VarType> Parse::Parser::ParseAngleExprList()
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

VarType Parse::Parser::ParseType()
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
    return symbol_->getVarType(type);
}

void Parser::print()
{
    for (auto& expr : declartions_) {
        expr->print("", true);
        std::cout << std::endl;
    }
}


//std::unique_ptr<Stmt> Parse::Parser::ParseMemberAccess(std::unique_ptr<Stmt> lhs,OperatorType Op)
//{
//   // auto type = lhs->getType();
//    auto name = cur_token_.content;
//    getNextToken();
//    if(cur_token_.type==TokenType::lParenthesis)
//    {
//        //Function F;
//        //auto args = ParseParenExprList();
//        //for (auto& f : symbol_->getClass(type.typeName).memberFunctions) {
//        //    if (f.name == name&&f.args.size()==args.size()) {
//        //        size_t i = 0;
//        //        bool flag = true;
//        //        while (i<args.size())
//        //        {
//        //            if(F.args[i].type!=args[i]->getType())
//        //            {
//        //                flag = false;
//        //                break;
//        //            }
//        //        }
//        //        if (flag) {
//        //            F = f; break;
//        //        }
//        //    }
//        //}
//        //if(F.name=="")
//        //{
//        //    error("No suitable function for " + type.typeName + ".");
//        //    return nullptr;
//        //}
//        auto call = std::make_unique<CallExprAST>(Function::mangle(F), std::move(args), F.returnType);
//        call->setThis(std::move(lhs));
//        return call;
//    }else
//    {
//        VarType retType;
//        int i = 0;
//        for(auto& m:symbol_->getClass(type.typeName).memberVariables)
//        {
//            if(m.name==name)
//            {
//                retType = m.type;
//                break;
//            }
//            ++i;
//        }
//        if(retType.typeName=="")
//        {
//            error("No '" + name + "' in class " + type.typeName + ".");
//            return nullptr;
//        }
//        return std::make_unique<MemberAccessAST>(std::move(lhs), i, retType);
//    }
//}

Function Parse::Parser::generateFunction_new(const VarType& t)
{
    Function f;
    f.name = "new";
    f.classType = t;
    VarType retType("__ptr");
    retType.templateArgs.push_back(t);
    f.returnType = retType;
    return f;
}

std::shared_ptr<SymbolTable> Parse::Parser::symbolTable()
{
    return symbol_;
}

void Parse::Parser::ParseExternal()
{
    isExternal = true;
    getNextToken();
    if (cur_token_.type != TokenType::Colon)
        error("Expected : after external.");
    getNextToken();
}
void Parse::Parser::ParseInternal() {
    isExternal = false;
    getNextToken();
    if (cur_token_.type != TokenType::Colon)
        error("Expected : after internal.");
    getNextToken();
}

void Parse::Parser::ParseTemplateClass()
{
    getNextToken();  // eat <
    std::vector<std::pair<VarType, std::string>> typelist;
    while (cur_token_.type!=TokenType::rAngle)
    {
        auto type = ParseType();
        auto name = cur_token_.content;
        getNextToken();
        typelist.emplace_back(type, name);
        if (cur_token_.type == TokenType::Comma) getNextToken();
    }
    getNextToken();  // eat >
    if(cur_token_.type==TokenType::Class)
    {
        std::vector<Token> tokenStream;
        tokenStream.push_back(cur_token_);
        getNextToken();
        if(cur_token_.type!=TokenType::Identifier)
        {
            error("Invalid identifier.");
            return;
        }
        auto name = cur_token_.content;
        tokenStream.push_back(cur_token_);
        getNextToken();
        if(cur_token_.type!=TokenType::lBrace)
        {
            error("Expected { .");
            return;
        }
        tokenStream.push_back(cur_token_);
       // getNextToken();
        int i = 1;
        while (true)
        {
            getNextToken();
            tokenStream.push_back(cur_token_);
            if (cur_token_.type == TokenType::lBrace) ++i;
            if (cur_token_.type == TokenType::rBrace) --i;
            if (i == 0) break;
        }
        ClassTemplate template_;
        template_.type = VarType(name);
        template_.token = std::move(tokenStream);
        template_.typeList = std::move(typelist);
        symbol_->addClassTemplate(std::move(template_));

    }else
    {
        error("Only class supports template.");
        return;
    }
}

void Parse::Parser::ParseUsing()
{
    getNextToken();  // eat using
    if (cur_token_.type != TokenType::Identifier)
    {
        error("Expected identifier after using.");
        return;
    }
    auto newType = cur_token_.content;
    getNextToken();
    if(cur_token_.type!=TokenType::Equal)
    {
        error("Expected = after using typename.");
        return;
    }
    getNextToken();  // eat =
    auto oldType = ParseType();
    auto c = symbol_->getClass(VarType::mangle(oldType));
    if(c.type.typeName=="")
    {
        error("Unknown type.");
        return;
    }
    symbol_->setAlias(VarType(newType), c.type);
}

Class Parse::Parser::InstantiateTemplate(VarType type,const ClassTemplate& template_)
{
    //SymbolTable::NamespaceGuard guard(*symbol_,"");
    if(type.templateArgs.size()!=template_.typeList.size())
    {
        error("Type arguments amount mismatch.");
        return {};
    }
    for(size_t i = 0;i<template_.typeList.size();++i)
    {
        symbol_->setAlias(VarType(template_.typeList[i].second), type.templateArgs[i]);
    }
    
    auto c = ParseClass((type));
    if (c == nullptr) return {};
    declartions_.push_back(std::move(c));
    //Class class_(type);
    return Class(type);
}

//std::unique_ptr<Stmt> Parser::callDestructor(const Variable& v)
//{
//    if(is_builtin_type(v.type.typeName))
//    {
//        return std::make_unique<NonExprAST>();
//    }
//    auto destructor = symbolTable()->getClass(v.type.typeName).destructor;
//    auto c = std::make_unique<CallExprAST>(destructor.mangledName(), std::vector<std::unique_ptr<Stmt>>(), VarType("void"));
//    c->setThis(std::make_unique<VariableExprAST>(v.name,v.type));
//    return c;
//}

//void Parser::generateDestructor(Class& c, std::unique_ptr<BlockExprAST> block)
//{
//    std::vector<std::unique_ptr<Stmt>> callDestructorForClassMember;
//    for(auto it=c.memberVariables.rbegin();it!=c.memberVariables.rend();++it)
//    {
//        callDestructorForClassMember.push_back(callDestructor(*it));
//    }
//    if (block == nullptr) {
//        block = std::make_unique<BlockExprAST>(std::move(callDestructorForClassMember), false);
//    } else {
//        block->instructions().insert(block->instructions().end(), std::make_move_iterator(callDestructorForClassMember.begin()),
//            std::make_move_iterator(callDestructorForClassMember.end()));
//    }
//    std::vector<Variable> args;
//    Function f("__destructor", args, VarType("void"), false);
//    f.classType = c.type;
//    std::vector<std::pair<std::string, std::string>> argList;
//    auto P = std::make_unique<PrototypeAST>(::Function::mangle(f), argList, "void");
//    P->setClassType(VarType::mangle(c.type));
//    auto F = std::make_unique<FunctionAST>(::Function::mangle(f), std::move(block), VarType::mangle(c.type));
//    proto_.push_back(std::move(P));
//    expr_.push_back(std::move(F));
//    c.destructor = f;
//}

