#include "Parser.h"
#include <stack>
#include <iostream>
#include <cassert>

using namespace Parse;

std::unique_ptr<Stmt> Parser::ParseIntegerExpr() {
    auto res = std::make_unique<IntegerStmt>(std::stoi(lexer_.curToken().content));
    getNextToken();
    return res;
}

std::unique_ptr<Stmt> Parser::ParseFloatExpr() {
    auto res = std::make_unique<FloatStmt>(std::stof(lexer_.curToken().content));
    getNextToken();
    return res;
}

std::unique_ptr<Stmt> Parse::Parser::ParseStatement() {
    if (lexer_.curToken().type == TokenType::Return) {
        return ParseReturnExpr();
    }
    if (lexer_.curToken().type == TokenType::If) {
        return ParseIfExpr();
    }
    if (lexer_.curToken().type == TokenType::For) {
        return ParseForExpr();
    }
    // TODO: Using statement
    //if(lexer_.curToken().type==TokenType::Using)
    //{
    //    ParseUsing();
    //    return std::make_unique<NonExprAST>();
    //}
    return ParseExpression();
}

std::unique_ptr<Stmt> Parse::Parser::ParsePrimary() {
    if (lexer_.curToken().type == TokenType::Identifier) {
        auto e = ParseIdentifierExpr();
        while (isPostOperator()) {
            e = ParsePostOperator(std::move(e));
        }
        return e;
    }
    if (lexer_.curToken().type == TokenType::Integer) {
        return ParseIntegerExpr();
    }
    if (lexer_.curToken().type == TokenType::Float) {
        return ParseFloatExpr();
    }
    if (lexer_.curToken().type == TokenType::lParenthesis) {
        return ParseParenExpr();
    }
    auto op = getNextUnaryOperator();
    if (op != OperatorType::None) {
        auto expr = ParsePrimary();
        return std::make_unique<UnaryOperatorStmt>(std::move(expr), op);
    }
    error("Unexpected token.");
    return nullptr;
}

bool Parse::Parser::isPostOperator() {
    if (lexer_.curToken().type == TokenType::lSquare) {
        return true;
    } else if (lexer_.curToken().type == TokenType::lParenthesis) {
        return true;
    } else if (lexer_.curToken().type == TokenType::Point) {
        return true;
    } else if (lexer_.curToken().type == TokenType::Plus) {
        return  lexer_.viewNextToken().type == TokenType::Plus;
    } else if (lexer_.curToken().type == TokenType::Minus) {
        return  lexer_.viewNextToken().type == TokenType::Minus || lexer_.viewNextToken().type == TokenType::rAngle;
    } else if (lexer_.curToken().type == TokenType::Colon) {
        return true;
    }
    return false;
}

std::unique_ptr<Stmt> Parse::Parser::ParsePostOperator(std::unique_ptr<Stmt> e) {
    //auto type = e->getType();
    if (lexer_.curToken().type == TokenType::lSquare) {
        //if (type.typeName != "__arr") {
        //    error("No suitable [] operator for " + type.typeName + ".");
        //    return nullptr;
        //}
        e = std::make_unique<UnaryOperatorStmt>(std::move(e),
            OperatorType::Subscript, ParseSquareExprList());
    } else if (lexer_.curToken().type == TokenType::lParenthesis) {
        e = std::make_unique<UnaryOperatorStmt>(std::move(e),
            OperatorType::FunctionCall, ParseParenExprList());
    } else if (lexer_.curToken().type == TokenType::Point) {
        getNextToken();
        if (lexer_.curToken().type != TokenType::Identifier) {
            error("Expected members of class.");
        }
        auto name = lexer_.curToken().content;
        getNextToken();
        e = std::make_unique<BinaryOperatorStmt>(std::move(e), std::make_unique<VariableStmt>(name),
            OperatorType::MemberAccessP);
        //e = ParseMemberAccess(std::move(e), OperatorType::MemberAccessP);
    } else if (lexer_.curToken().type == TokenType::Plus) {
        assert(getNextUnaryOperator() == OperatorType::PreIncrement);
        e = std::make_unique<UnaryOperatorStmt>(std::move(e), OperatorType::PostIncrement);
    } else if (lexer_.curToken().type == TokenType::Minus) {
        if (lexer_.viewNextToken().type == TokenType::rAngle) {
            //assert(e->getType().typeName == "__ptr");
            assert(getNextBinOperator() == OperatorType::MemberAccessA);
            e = std::make_unique<BinaryOperatorStmt>(std::move(e), ParseExpression(), OperatorType::MemberAccessA);
            //e = ParseMemberAccess(std::move(e), OperatorType::MemberAccessP);
        } else {
            assert(getNextUnaryOperator() == OperatorType::PreDecrement);
            e = std::make_unique<UnaryOperatorStmt>(std::move(e), OperatorType::PostDecrement);
        }

    }
    // TODO: Namspace
    /* else if(lexer_.curToken().type==TokenType::Colon)
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
    if (lexer_.curToken().type != TokenType::rParenthesis) {
        error("Expected ).");
        return nullptr;
    }
    getNextToken(); //eat )
    return expr;
}

std::unique_ptr<Stmt> Parse::Parser::ParseIdentifierExpr() {
    auto idname = lexer_.curToken().content;
    getNextToken();
    // is defining a identifier
 //   if (lexer_.curToken().type_llvm == TokenType::Identifier||lexer_.curToken().type_llvm==TokenType::lAngle) {
    if (lexer_.curToken().type == TokenType::Identifier || lexer_.curToken().type == TokenType::lAngle || lexer_.curToken().type == TokenType::lParenthesis) {
        auto cur_it = lexer_.getIterator();
        auto def = ParseVariableDefinition(idname);
        if(def)
        {
            return def;
        }
        lexer_.setIterator(cur_it);
    }
    // TODO: Namspace
    //if(lexer_.curToken().type==TokenType::Colon)
    //{
    //    return std::make_unique<NamespaceExprAST>(idname);
    //}
    // is only a identifier
    //if (lexer_.curToken().type != TokenType::lParenthesis) {
        //auto var = symbol_->getValue(idname);
        //if(var.name=="")
        //{
        //    error("Unknown identifier.");
        //    return nullptr;
        //}
    return std::make_unique<VariableStmt>(idname);;
    //}
    // TODO: is calling a function
    /*assert(lexer_.curToken().type == TokenType::lParenthesis);
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
    getNextToken(); //eat for
    if (lexer_.curToken().type != TokenType::lParenthesis) {
        error("Expected ( after a for loop.");
        return nullptr;
    }
    getNextToken(); //eat (
    auto start = ParseExpression();
    if (lexer_.curToken().type != TokenType::Semicolon) {
        error("Expected ; to split for expression.");
        return nullptr;
    }
    getNextToken(); //eat ;
    auto cond = ParseExpression();
    if (lexer_.curToken().type != TokenType::Semicolon) {
        error("Expected ; to split for expression.");
        return nullptr;
    }
    getNextToken(); //eat ;
    auto end = ParseExpression();
    if (lexer_.curToken().type != TokenType::rParenthesis) {
        error("Expected ) to end for condition.");
        return nullptr;
    }
    getNextToken(); //eat )
    auto body = ParseBlock();
    //sg.setBlock(body.get());
    return std::make_unique<ForStmt>(std::move(start), std::move(cond), std::move(end), std::move(body));
}

std::unique_ptr<Stmt> Parse::Parser::ParseVariableDefinition(const std::string& type_name) {
    std::vector<std::unique_ptr<Stmt>> template_args;
    if (lexer_.curToken().type == TokenType::lAngle) {
        template_args = ParseAngleExprList();
    }
    auto type = std::make_unique<TypeStmt>(type_name, std::move(template_args));
    if (lexer_.curToken().type == TokenType::Identifier) {   // like 'int p=0'
        auto varname = lexer_.curToken().content;
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
        auto expr = std::make_unique<VariableDefStmt>(std::move(type), varname);
        //if (type_name == "__arr" || type_name == "__ptr") {
        //    std::vector<std::string> tempargs;
        //    for (auto& v : type.templateArgs) {
        //        tempargs.push_back(::VarType::mangle(v));
        //    }
        //    expr->setTemplateArgs(tempargs);
        //}
        if (lexer_.curToken().type == TokenType::Equal) {
            // definition with initiate value
            getNextToken();   // eat =
            auto E = ParseExpression();
            expr->setInitValue(std::move(E));
        } else if (lexer_.curToken().type != TokenType::Semicolon) {
            error("Expected initiate value or ;.");
            return nullptr;
        }
        return expr;
    } else {   // like 'Var(10)'
        if(lexer_.curToken().type != TokenType::lParenthesis)
        {
            return nullptr;
        }
        auto args = ParseParenExprList();

        return std::make_unique<UnaryOperatorStmt>(std::move(type),OperatorType::FunctionCall, std::move(args));
    }
}

std::unique_ptr<Stmt> Parse::Parser::ParseReturnExpr() {
    getNextToken();
    std::unique_ptr<Stmt> retval = nullptr;
    if (lexer_.curToken().type != TokenType::Semicolon) {
        retval = ParseExpression();
        if (!retval) {
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

std::unique_ptr<Stmt> Parse::Parser::MergeExpr(std::unique_ptr<Stmt> LHS, std::unique_ptr<Stmt> RHS, OperatorType Op) {
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
    while (true) {
        OperatorType op;
        if (lexer_.curToken().type == TokenType::Semicolon || lexer_.curToken().type == TokenType::rParenthesis
            || lexer_.curToken().type == TokenType::rSquare || lexer_.curToken().type == TokenType::Comma) {
            if (ops.top() == OperatorType::None) break;
            op = OperatorType::None;
        } else
            op = getNextBinOperator();
        while (getBinOperatorPrecedence(op) < getBinOperatorPrecedence(ops.top())) {
            std::stack<OperatorType> curLevelOps;
            std::stack<std::unique_ptr<Stmt>> curLevelExprs;
            curLevelExprs.push(std::move(expr.top()));
            expr.pop();
            auto curPrecedence = getBinOperatorPrecedence(ops.top());
            while (curPrecedence == getBinOperatorPrecedence(ops.top())) {
                curLevelOps.push(ops.top());
                ops.pop();
                curLevelExprs.push(std::move(expr.top()));
                expr.pop();
            }
            while (!curLevelOps.empty()) {
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
            assert(curLevelExprs.empty() == true);
        }
        if (op != OperatorType::None) {
            ops.push(op);
            expr.push(ParsePrimary());
        }
    }
    return std::move(expr.top());
}

OperatorType Parse::Parser::getNextBinOperator() {
    auto ret = [&](OperatorType o) {
        getNextToken();
        return o;
    };
    TokenType first = lexer_.curToken().type;
    getNextToken();
    if (first == TokenType::Colon) {
        if (lexer_.curToken().type == TokenType::Colon)   // ::
            return ret(OperatorType::ScopeResolution);
        else
            return OperatorType::None;
    }
    if (first == TokenType::Minus) {
        if (lexer_.curToken().type == TokenType::rAngle)  // ->
            return ret(OperatorType::MemberAccessA);
        if (lexer_.curToken().type == TokenType::Equal)   // -=
            return ret(OperatorType::MinComAssign);
        return OperatorType::Subtraction;
    }
    if (first == TokenType::lAngle) {
        if (lexer_.curToken().type == TokenType::lAngle) {  // <<
            getNextToken();
            if (lexer_.curToken().type == TokenType::Equal)    // <<=
                return ret(OperatorType::LshComAssign);
            return OperatorType::LeftShift;
        }
        if (lexer_.curToken().type == TokenType::Equal)      // <=
            return ret(OperatorType::LessEqual);
        return OperatorType::Less;
    }
    if (first == TokenType::rAngle) {
        if (lexer_.curToken().type == TokenType::rAngle)         // >>
        {
            getNextToken();
            if (lexer_.curToken().type == TokenType::Equal)   // >>=
                return ret(OperatorType::RshComAssign);
            return OperatorType::RightShift;
        }
        if (lexer_.curToken().type == TokenType::Equal)       // >=
            return ret(OperatorType::GreaterEqual);
        return OperatorType::Greater;
    }
    if (first == TokenType::Exclam) {
        if (lexer_.curToken().type == TokenType::Equal)       // !=
            return ret(OperatorType::NotEqual);
        return OperatorType::None;
    }
    if (first == TokenType::Multiply) {
        if (lexer_.curToken().type == TokenType::Equal)        // *=
            return ret(OperatorType::MulComAssign);
        return OperatorType::Multiplication;
    }
    if (first == TokenType::Divide) {
        if (lexer_.curToken().type == TokenType::Equal)
            return ret(OperatorType::DivComAssign);
        return OperatorType::Division;
    }
    if (first == TokenType::Plus) {
        if (lexer_.curToken().type == TokenType::Equal)
            return ret(OperatorType::SumComAssign);
        return OperatorType::Addition;
    }
    if (first == TokenType::Percent) {
        if (lexer_.curToken().type == TokenType::Equal)
            return ret(OperatorType::RemComAssign);
        return OperatorType::Remainder;
    }
    if (first == TokenType::And) {
        if (lexer_.curToken().type == TokenType::And)
            return ret(OperatorType::LogicalAND);
        if (lexer_.curToken().type == TokenType::Equal)
            return ret(OperatorType::ANDComAssign);
        return ret(OperatorType::BitwiseAND);
    }
    if (first == TokenType::Or) {
        if (lexer_.curToken().type == TokenType::Or)
            return ret(OperatorType::LogicalOR);
        if (lexer_.curToken().type == TokenType::Equal)
            return ret(OperatorType::ORComAssign);
        return OperatorType::BitwiseOR;
    }
    if (first == TokenType::Xor) {
        if (lexer_.curToken().type == TokenType::Xor)
            return ret(OperatorType::XORComAssign);
        return OperatorType::None;
    }
    if (first == TokenType::Equal) {
        if (lexer_.curToken().type == TokenType::Equal)
            return ret(OperatorType::Equal);
        return OperatorType::Assignment;
    }
    return TokenToBinOperator(first);
}

OperatorType Parse::Parser::getNextUnaryOperator() {
    auto ret = [&](OperatorType o) {
        getNextToken();
        return o;
    };
    TokenType first = lexer_.curToken().type;
    getNextToken();
    if (first == TokenType::Plus) {
        if (lexer_.curToken().type == TokenType::Plus)
            return ret(OperatorType::PreIncrement);
        return OperatorType::Promotion;
    }
    if (first == TokenType::Minus) {
        if (lexer_.curToken().type == TokenType::Minus)
            return ret(OperatorType::PreDecrement);
        return OperatorType::Negation;
    }
    if (first == TokenType::Exclam) {
        return OperatorType::LogicalNOT;
    }
    if (first == TokenType::Tilde) {
        return OperatorType::BitwiseNOT;
    }
    if (first == TokenType::Multiply) {
        return OperatorType::Dereference;
    }
    return OperatorType::None;
}

std::unique_ptr<Stmt> Parse::Parser::ParseIfExpr() {
    getNextToken();  // eat if
    auto cond = ParseParenExpr();
    if (!cond) return nullptr;
    //   if (lexer_.curToken().type_llvm != TokenType::lBrace)
    //       return LogError("Expected {.");
    auto then = ParseBlock();
    if (lexer_.curToken().type == TokenType::Else) {
        getNextToken();
        if (lexer_.curToken().type == TokenType::If) {
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

std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> Parser::ParseFunctionArgList() {
    std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> ArgNames;
    getNextToken();
    while (lexer_.curToken().type != TokenType::rParenthesis) {
        auto type = ParseType();
        // getNextToken();
        ArgNames.emplace_back(std::make_pair(std::move(type), lexer_.curToken().content));
        getNextToken();
        if (lexer_.curToken().type == TokenType::rParenthesis) break;
        if (lexer_.curToken().type != TokenType::Comma) {
            error("Expected , to split arguments.");
            return {};
        }
        getNextToken();
    }
    return ArgNames;
}


std::unique_ptr<FunctionDecl> Parse::Parser::ParsePrototype() {
    if (lexer_.curToken().type != TokenType::Identifier) {
        error("Expected function name in prototype");
        return nullptr;
    }
    std::string FnName = lexer_.curToken().content;
    getNextToken();
    if (lexer_.curToken().type != TokenType::lParenthesis) {
        error("Expected '(' in prototype");
        return nullptr;
    }
    // Read the list of argument names.
    auto ArgNames = ParseFunctionArgList();

    if (lexer_.curToken().type != TokenType::rParenthesis) {
        error("Expected ')' in prototype");
        return nullptr;
    }

    getNextToken(); // eat ')'.
    std::unique_ptr<Stmt> retType = nullptr;
    if (lexer_.curToken().type == TokenType::Minus) {
        auto op = getNextBinOperator();
        if (op != OperatorType::MemberAccessA) {
            error("Unexpected symbol.");
            return nullptr;
        }
        if (lexer_.curToken().type != TokenType::Identifier) {
            error("Unknown type.");
            return nullptr;
        }
        retType = ParseType();
    } else if (lexer_.curToken().type != TokenType::lBrace) {
        error("Return value must be pointed out explicitly when declaring a function prototype.");
        return nullptr;
    } else {
        retType = std::make_unique<TypeStmt>("void");
    }

    //auto p = std::make_unique<PrototypeAST>(::Function::mangle(f),std::move(argList),::VarType::mangle(retType));
    //if (cur_class_.typeName != "")
    //    p->setClassType(::VarType::mangle(cur_class_));
    //proto_.emplace_back(std::move(p));
    //symbol_->addFunction(f);
    return std::make_unique<FunctionDecl>(FnName, std::move(ArgNames), std::move(retType), nullptr, isExternal);
}

std::unique_ptr<CompoundStmt> Parse::Parser::ParseBlock() {
    std::vector<std::unique_ptr<Stmt>> body;
    bool hasReturn = false;
    if (lexer_.curToken().type != TokenType::lBrace) {   // with only one statement, {} can be omitted
        auto expr = ParseStatement();
        if (expr != nullptr) body.emplace_back(std::move(expr));
        //if (dynamic_cast<ReturnStmt*>(expr.get()) != nullptr) hasReturn = true;
        if (lexer_.curToken().type == TokenType::Semicolon) {
            getNextToken();  //eat ;
        }
        //symbol_->callNamelessVarDestructor(body);
    } else {   // block with { }
        getNextToken();      //eat {
        while (lexer_.curToken().type != TokenType::rBrace && lexer_.curToken().type != TokenType::Eof) {
            auto E = ParseStatement();
            if (!E) return nullptr;
            //if (dynamic_cast<ReturnAST*>(E.get()) != nullptr) hasReturn = true;
            body.emplace_back(std::move(E));
            if (lexer_.curToken().type == TokenType::Semicolon)
                getNextToken();     // eat ;
            //symbolTable()->callNamelessVarDestructor(body);
        }
        if (lexer_.curToken().type != TokenType::rBrace) {
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

std::unique_ptr<FunctionDecl> Parse::Parser::ParseFunction() {
    getNextToken(); // eat fn.
    auto f = ParsePrototype();
    if (!f) return {};
    std::unique_ptr<CompoundStmt> body;

    if (lexer_.curToken().type == TokenType::Semicolon) {
        getNextToken();
        f->setBody(nullptr);
        return f;
    }
    if (lexer_.curToken().type != TokenType::lBrace) {
        error("Expected { or ;.");
        return {};
    }
    body = ParseBlock();
    if (!body) {
        return {};
    }
    f->setBody(std::move(body));
    //if(!body->hasReturn()&&dynamic_cast<ReturnAST*>(body->instructions().back().get())==nullptr)
    //{
    //    body->instructions().push_back(std::make_unique<ReturnAST>(nullptr, symbolTable()->callDestructor()));
    //    body->setHasReturn();
    //}
  //  sg.setBlock(body.get());

//declartions_.push_back();
    return f;
}

void Parse::Parser::HandleDefinition() {
    auto func = ParseFunction();
    if (func != nullptr) {
        fprintf(stderr, "Parsed a function definition.\n");
        functionDecls_.emplace_back(std::move(func));
        //expr_.emplace_back(std::move(func));
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

std::unique_ptr<ClassDecl> Parse::Parser::ParseClass() {
    getNextToken();  // eat class
    if (lexer_.curToken().type != TokenType::Identifier) {
        error("Expected class name.");
        return nullptr;
    }
    auto classDecl = std::make_unique<ClassDecl>(lexer_.curToken().content);
    getNextToken();
    if (lexer_.curToken().type != TokenType::lBrace) {
        error("Expect class body.");
        return nullptr;
    }
    getNextToken(); // eat ;
    //std::unique_ptr<CompoundStmt> desturctorBlock = nullptr;
    while (lexer_.curToken().type != TokenType::rBrace) {
        if (lexer_.curToken().type == TokenType::Identifier) {
            if (lexer_.curToken().content == classDecl->name() && lexer_.viewNextToken().type == TokenType::lParenthesis) {
                // parsing constructor
                getNextToken();
                auto ArgNames = ParseFunctionArgList();
                getNextToken();
                std::unique_ptr<CompoundStmt> b = ParseBlock();
                auto retType = std::make_unique<TypeStmt>("void");
                auto F = std::make_unique<FunctionDecl>("__construct", std::move(ArgNames), std::move(retType), std::move(b));
                classDecl->addConstructor(std::move(F));
            } else {   // parsing member variable
                auto type = ParseType();
                auto name = lexer_.curToken().content;
                getNextToken();
                if (name == "") {
                    error("Invalid declaration.");
                    return nullptr;
                }
                //symbol_->setValue(name, Variable(name, type));
                classDecl->addMemberVariable(std::move(type), name);
                //c.memberVariables.emplace_back(name, type);
            }
        } else if (lexer_.curToken().type == TokenType::Function) {
            auto f = ParseFunction();
            classDecl->addMemberFunction(std::move(f));
        } else if (lexer_.curToken().type == TokenType::Tilde) {   // parsing destructor
            if (getNextToken().type != TokenType::Identifier || lexer_.curToken().content != classDecl->name()) {
                error("Expected classname to identify destructor.");
                return nullptr;
            }
            getNextToken();
            auto exprs = ParseParenExprList();
            if (exprs.size() != 0) {
                error("Destructor doesn't take any argument.");
                return nullptr;
            }
            auto b = ParseBlock();
            auto retType = std::make_unique<TypeStmt>("void");
            auto destructor = std::make_unique<FunctionDecl>("__destructor",
                std::vector<std::pair<std::unique_ptr<Stmt>, std::string>>{},
                std::move(retType), std::move(b));
            classDecl->setDestructor(std::move(destructor));
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
        if (lexer_.curToken().type == TokenType::Semicolon) getNextToken();

        //symbol_->addFunction(generateFunction_new(c.type));
        //generateDestructor(c, std::move(desturctorBlock));
        //cur_class_ = tmp;
    }
    getNextToken();
    return classDecl;
}

void Parse::Parser::HandleClass() {
    auto c = ParseClass();
    if (c) {
        fprintf(stderr, "Parsed a class.\n");
        classDecls_.push_back(std::move(c));
    } else {
        getNextToken();
    }
}

void Parse::Parser::MainLoop() {
    while (1) {
        switch (lexer_.curToken().type) {
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
            //case TokenType::Using:
            //    ParseUsing();
            //    break;
            //case TokenType::lAngle:
            //    ParseTemplateClass();
            //    break;
        default:
            getNextToken();
            //    HandleTopLevelExpression();
            break;
        }
    }
}

Token& Parse::Parser::getNextToken() {
    return lexer_.curToken() = lexer_.nextToken();
}

void Parse::Parser::error(const std::string& errmsg) {
    std::cout << lexer_.curToken().lineNum << "." << lexer_.curToken().charNum << ":\t";
    std::cout << errmsg << std::endl;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseExprList(TokenType endToken) {
    std::vector<std::unique_ptr<Stmt>> exprs;
    while (lexer_.curToken().type != endToken) {
        exprs.push_back(ParseExpression());
        if (lexer_.curToken().type == endToken) break;
        if (lexer_.curToken().type != TokenType::Comma) {
            error("Expect , to split expressions.");
            return std::vector<std::unique_ptr<Stmt>>();
        }
        getNextToken();  // eat ,
    }
    return exprs;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseParenExprList() {
    getNextToken(); // eat (
    auto exprs = ParseExprList(TokenType::rParenthesis);
    getNextToken();  // eat )
    return exprs;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseSquareExprList() {
    getNextToken(); // eat [
    auto exprs = ParseExprList(TokenType::rSquare);
    getNextToken();  // eat ]
    return exprs;
}

std::vector<std::unique_ptr<Stmt>> Parse::Parser::ParseAngleExprList() {
    getNextToken(); // eat <
    //auto exprs = ParseExprList(TokenType::rAngle);
    std::vector<std::unique_ptr<Stmt>> exprs;
    while (lexer_.curToken().type != TokenType::rAngle) {
        exprs.push_back(ParseType());
        if (lexer_.curToken().type == TokenType::Comma) {
            getNextToken();
        } else if (lexer_.curToken().type != TokenType::rAngle) {
            error("Expect , to split typename or > to end list.");
            return {};
        }
    }
    getNextToken();  // eat >
    return exprs;
}

std::unique_ptr<Stmt> Parse::Parser::ParseType() {
    /*bool isconst = false;
    if(lexer_.curToken().content=="const")
    {
        isconst = true;
    }*/
    auto type = lexer_.curToken().content;
    getNextToken();
    if (lexer_.curToken().type == TokenType::lAngle) {
        auto templateArgs = ParseAngleExprList();
        return std::make_unique<TypeStmt>(type, std::move(templateArgs));
    }
    return std::make_unique<TypeStmt>(type);
}

void Parser::print() {
    for (auto& expr : classDecls_) {
        expr->print("", true);
        std::cout << std::endl;
    }
    for (auto& expr : functionDecls_) {
        expr->print("", true);
        std::cout << std::endl;
    }
}

void Parser::convertToLLVM() {
    for (auto& expr : classDecls_) {
        expr->toLLVM(&context_);
    }
    for(auto& clas:classDecls_) {
        clas->registerMemberFunction(&context_);
    }
    for (auto& func : functionDecls_) {
        func->registerPrototype(&context_);
    }
    for(auto& func:functionDecls_)
    {
        func->toLLVM(&context_);
    }
}

ASTContext& Parser::context()
{
    return context_;
}


//std::unique_ptr<Stmt> Parse::Parser::ParseMemberAccess(std::unique_ptr<Stmt> lhs,OperatorType Op)
//{
//   // auto type = lhs->getType();
//    auto name = lexer_.curToken().content;
//    getNextToken();
//    if(lexer_.curToken().type==TokenType::lParenthesis)
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

void Parse::Parser::ParseExternal() {
    isExternal = true;
    getNextToken();
    if (lexer_.curToken().type != TokenType::Colon)
        error("Expected : after external.");
    getNextToken();
}
void Parse::Parser::ParseInternal() {
    isExternal = false;
    getNextToken();
    if (lexer_.curToken().type != TokenType::Colon)
        error("Expected : after internal.");
    getNextToken();
}

//void Parse::Parser::ParseTemplateClass()
//{
//    getNextToken();  // eat <
//    std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> typelist;
//    while (lexer_.curToken().type!=TokenType::rAngle)
//    {
//        auto type = ParseType();
//        auto name = lexer_.curToken().content;
//        getNextToken();
//        typelist.emplace_back(type, name);
//        if (lexer_.curToken().type == TokenType::Comma) getNextToken();
//    }
//    getNextToken();  // eat >
//    if(lexer_.curToken().type==TokenType::Class)
//    {
//        std::vector<Token> tokenStream;
//        tokenStream.push_back(lexer_.curToken());
//        getNextToken();
//        if(lexer_.curToken().type!=TokenType::Identifier)
//        {
//            error("Invalid identifier.");
//            return;
//        }
//        auto name = lexer_.curToken().content;
//        tokenStream.push_back(lexer_.curToken());
//        getNextToken();
//        if(lexer_.curToken().type!=TokenType::lBrace)
//        {
//            error("Expected { .");
//            return;
//        }
//        tokenStream.push_back(lexer_.curToken());
//       // getNextToken();
//        int i = 1;
//        while (true)
//        {
//            getNextToken();
//            tokenStream.push_back(lexer_.curToken());
//            if (lexer_.curToken().type == TokenType::lBrace) ++i;
//            if (lexer_.curToken().type == TokenType::rBrace) --i;
//            if (i == 0) break;
//        }
//        //ClassTemplate template_;
//        //template_.type = VarType(name);
//        //template_.token = std::move(tokenStream);
//        //template_.typeList = std::move(typelist);
//        //symbol_->addClassTemplate(std::move(template_));
//
//    }else
//    {
//        error("Only class supports template.");
//        return;
//    }
//}

//void Parse::Parser::ParseUsing()
//{
//    getNextToken();  // eat using
//    if (lexer_.curToken().type != TokenType::Identifier)
//    {
//        error("Expected identifier after using.");
//        return;
//    }
//    auto newType = lexer_.curToken().content;
//    getNextToken();
//    if(lexer_.curToken().type!=TokenType::Equal)
//    {
//        error("Expected = after using typename.");
//        return;
//    }
//    getNextToken();  // eat =
//    auto oldType = ParseType();
//    auto c = symbol_->getClass(VarType::mangle(oldType));
//    if(c.type.typeName=="")
//    {
//        error("Unknown type.");
//        return;
//    }
//    symbol_->setAlias(VarType(newType), c.type);
//}

//Class Parse::Parser::InstantiateTemplate(VarType type,const ClassTemplate& template_)
//{
//    //SymbolTable::NamespaceGuard guard(*symbol_,"");
//    if(type.templateArgs.size()!=template_.typeList.size())
//    {
//        error("Type arguments amount mismatch.");
//        return {};
//    }
//    for(size_t i = 0;i<template_.typeList.size();++i)
//    {
//        symbol_->setAlias(VarType(template_.typeList[i].second), type.templateArgs[i]);
//    }
//    
//    auto c = ParseClass((type));
//    if (c == nullptr) return {};
//    declartions_.push_back(std::move(c));
//    //Class class_(type);
//    return Class(type);
//}

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

