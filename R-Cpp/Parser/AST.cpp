#include "AST.h"

Parse::CompoundStmt::CompoundStmt(std::vector<std::unique_ptr<Stmt>> exprs)
    : stmts_(std::move(exprs)) 
{ }

void Parse::CompoundStmt::print(std::string indent, bool last) {
    indent += last ? "  " : "| ";
    for (size_t i = 0; i < stmts_.size(); ++i) {
        stmts_[i]->print(indent, i == stmts_.size() - 1);
    }
}

std::unique_ptr<ExprAST> Parse::CompoundStmt::toLLVMAST(ASTContext* context)
{
    return toBlockExprAST(context);
}

std::unique_ptr<BlockExprAST> Parse::CompoundStmt::toBlockExprAST(ASTContext* context)
{
    std::vector<std::unique_ptr<ExprAST>> exprs;
    bool hasReturn = false;
    for(auto&v:stmts_)
    {
        auto e = v->toLLVMAST(context);
        if (dynamic_cast<ReturnAST*>(e.get())) hasReturn = true;
        exprs.push_back(std::move(e));
    }
    return std::make_unique<BlockExprAST>(std::move(exprs), hasReturn);
}

Parse::IfStmt::IfStmt(std::unique_ptr<Stmt> condition, std::unique_ptr<CompoundStmt> then,
                      std::unique_ptr<CompoundStmt> els): cond_(std::move(condition)), then_(std::move(then)),
                                                          else_(std::move(els)) {
}

void Parse::IfStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-IfStmt " << std::endl;
    indent += last ? "  " : "| ";
    std::cout << indent << "+- Condition" << std::endl;
    cond_->print(indent, false);
    std::cout << indent << "+- Then" << std::endl;
    then_->print(indent, else_ == nullptr);
    if (else_) {
        std::cout << indent << "+- Else" << std::endl;
        else_->print(indent, true);
    }

}

std::unique_ptr<ExprAST> Parse::IfStmt::toLLVMAST(ASTContext* context)
{
    return std::make_unique<IfExprAST>(cond_->toLLVMAST(context), then_->toBlockExprAST(context),
        else_->toBlockExprAST(context));
}

Parse::ForStmt::ForStmt(std::unique_ptr<Stmt> start, std::unique_ptr<Stmt> cond, std::unique_ptr<Stmt> end,
                        std::unique_ptr<CompoundStmt> body): start_(std::move(start)), cond_(std::move(cond)),
                                                             end_(std::move(end)), body_(std::move(body)) {

}

void Parse::ForStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-ForStmt" << std::endl;
    indent += last ? "  " : "| ";
    std::cout << indent << "+-start" << std::endl;
    start_->print(indent + "| ", false);
    std::cout << indent << "+-cond" << std::endl;
    cond_->print(indent + "| ", false);
    std::cout << indent << "+-body" << std::endl;
    body_->print(indent + "| ", false);
    std::cout << indent << "+-end" << std::endl;
    end_->print(indent + "  ", true);
}

std::unique_ptr<ExprAST> Parse::ForStmt::toLLVMAST(ASTContext* context)
{
    return std::make_unique<ForExprAST>(start_->toLLVMAST(context), cond_->toLLVMAST(context),
        end_->toLLVMAST(context), body_->toBlockExprAST(context));
    
}

Parse::ReturnStmt::ReturnStmt(std::unique_ptr<Stmt> returnVal): ret_val_(std::move(returnVal)) {
}

void Parse::ReturnStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-ReturnStmt" << std::endl;
    indent += last ? "  " : " |";
    ret_val_->print(indent, true);
}

std::unique_ptr<ExprAST> Parse::ReturnStmt::toLLVMAST(ASTContext* context)
{
    return std::make_unique<ReturnAST>(ret_val_->toLLVMAST(context), std::vector<std::unique_ptr<ExprAST>>{});
}

Parse::BinaryOperatorStmt::
BinaryOperatorStmt(std::unique_ptr<Stmt> lhs, std::unique_ptr<Stmt> rhs, OperatorType op): lhs_(std::move(lhs)),
                                                                                           rhs_(std::move(rhs)),
                                                                                           op_(op) {
}

void Parse::BinaryOperatorStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-BinaryOperatorStmt " << op_ << std::endl;
    indent += last ? "  " : "| ";
    lhs_->print(indent, false);
    rhs_->print(indent, true);
}

std::unique_ptr<ExprAST> Parse::BinaryOperatorStmt::toLLVMAST(ASTContext* context)
{
    auto l = lhs_->toLLVMAST(context);
    auto r = rhs_->toLLVMAST(context);
    if(lhs_->getType()!=rhs_->getType())
    {
        throw std::logic_error("No suitable binary operator between "+lhs_->getType()->getTypename()+" and "+rhs_->getType()->getTypename()+".");
    }
    return std::make_unique<BinaryExprAST>(op_, std::move(l), std::move(r),l->getType());
}

Parse::UnaryOperatorStmt::
UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op): stmt_(std::move(expr)), op_(op) {
}

Parse::UnaryOperatorStmt::UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op,
                                            std::vector<std::unique_ptr<Stmt>> args): stmt_(std::move(expr)), op_(op),
                                                                                      args_(std::move(args)) {
}

void Parse::UnaryOperatorStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-UnaryOperatorStmt " << op_ << std::endl;
    indent += last ? "  " : "| ";
    if (args_.size() == 0) {
        stmt_->print(indent, true);
    } else {
        stmt_->print(indent, false);
        for (size_t i = 0; i < args_.size(); ++i) {
            args_[i]->print(indent, i == args_.size() - 1);
        }
    }
}

std::unique_ptr<ExprAST> Parse::UnaryOperatorStmt::toLLVMAST(ASTContext* context) {
    auto expr = stmt_->toLLVMAST(context);
    std::vector<std::unique_ptr<ExprAST>> argsExpr;
    for(auto&arg:args_)
    {
        argsExpr.push_back(arg->toLLVMAST(context));
    }
    if(expr==nullptr&&op_==OperatorType::FunctionCall) {

        auto fnName = dynamic_cast<VariableStmt*>(stmt_.get());
        if(!fnName)
        {
            throw std::logic_error("Is not a callable object.");
        }
        auto fnList = context->symbolTable().getFunction(fnName->getName());
        FunctionType* target=nullptr;
        for (auto& f : *fnList) {
            bool flag = true;
            if (f->args().size() == args_.size()) {
                size_t i = 0;
                while (i < f->args().size()) {
                    if (f->args()[i].first != args_[i]->getType()) {
                        flag = false;
                        break;
                    }
                    ++i;
                }
                if (flag) target = f.get();
            }
        }
        if (!target) {
            throw std::logic_error("No suitable function.");
        }
        return std::make_unique<CallExprAST>(target->mangledName(), std::move(argsExpr), target->returnType()->mangledName());
    }
}

Parse::VariableDefStmt::VariableDefStmt(std::unique_ptr<Stmt> type, const std::string& name)
    : vartype_(std::move(type)), name_(name)
{ }

void Parse::VariableDefStmt::setInitValue(std::unique_ptr<Stmt> initVal) {
    init_val_ = std::move(initVal);
}

void Parse::VariableDefStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-VariableDefStmt " << name_ << " ";
    vartype_->print(indent, last);
    std::cout << std::endl;
    indent += last ? "  " : "| ";
    if (init_val_)
        init_val_->print(indent, true);
}

std::unique_ptr<ExprAST> Parse::VariableDefStmt::toLLVMAST(ASTContext* context) {
    vartype_->toLLVMAST(context);
    auto t = dynamic_cast<TypeStmt*>(vartype_.get());
    if (t == nullptr)
        throw std::logic_error("Invalid type.");
    if(context->symbolTable().getVariable(name_)!=nullptr) {
        throw std::logic_error("Duplicate variable name: " + name_);
    }
    context->symbolTable().addVariable(t->getType(), name_);
    if(init_val_!=nullptr) {
        return std::make_unique<VariableDefAST>(t->getType()->mangledName(), name_, init_val_->toLLVMAST(context));
    }else {
        return std::make_unique<VariableDefAST>(t->getType()->mangledName(), name_);
    }

}

Parse::NamelessVariableStmt::
NamelessVariableStmt(std::unique_ptr<Stmt> type,
                     std::vector<std::unique_ptr<Stmt>> args): vartype_(std::move(type)), args_(std::move(args)) {
}

void Parse::NamelessVariableStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-NamelessVariableStmt ";
    vartype_->print(indent, last);
    std::cout << std::endl;
    indent += last ? "  " : "| ";
    for (size_t i = 0; i < args_.size(); ++i) {
        args_[i]->print(indent, i == args_.size() - 1);
    }
}

std::unique_ptr<ExprAST> Parse::NamelessVariableStmt::toLLVMAST(ASTContext* context) {
    vartype_->toLLVMAST(context);
    auto t = dynamic_cast<TypeStmt*>(vartype_.get());
    if(!t)
    {
        throw std::logic_error("Invalid type.");
    }
    FunctionType* target=nullptr;
    auto type = dynamic_cast<CompoundType*>(t->getType());
    if(!type)
    {
        throw std::logic_error("No suitable constructor.");
    }
    for (auto& f : type->getConstructors()) {
        if (f->args().size() == args_.size()) {
            bool flag = true;
            for (size_t i = 0; i < f->args().size(); ++i) {
                if (f->args()[i].first != args_[i]->getType()) {
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
    if (!target) {
        throw std::logic_error("No suitable constructor.");
    }
    std::vector<std::unique_ptr<Stmt>> exprlist;
    std::string varname = "__" + std::to_string(context->getNamelessVarCount()) + "tmp_";
    context->symbolTable().addNamelessVariable(t->getType(),varname);
}

Parse::VariableStmt::VariableStmt(const std::string& name): name_(name) {
}

void Parse::VariableStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-VariableStmt " << name_ << std::endl;
}

std::unique_ptr<ExprAST> Parse::VariableStmt::toLLVMAST(ASTContext* context) {
    auto v = context->symbolTable().getVariable(name_);
    if (v) {  // is a variable
        type_ = v->type_;
        return std::make_unique<VariableExprAST>(name_, v->type_->mangledName());
    }
    auto f = context->symbolTable().getFunction(name_);
    if(!f)  // is not a function
        throw std::logic_error("Unknown identifier: " + name_);
    //type_ = v.type;
    return nullptr;
}

Parse::IntegerStmt::IntegerStmt(std::int64_t val): val_(val) {
}

void Parse::IntegerStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-IntegerStmt " << val_ << std::endl;
}

Parse::FloatStmt::FloatStmt(double val): val_(val) {
}

void Parse::FloatStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-FloatStmt " << val_ << std::endl;
}

Parse::FunctionDecl::FunctionDecl(std::string funcName,
    std::vector<std::pair<std::unique_ptr<Stmt>, std::string>> args,
    std::unique_ptr<Stmt> retType, std::unique_ptr<CompoundStmt> body,bool isExternal)
: funcName_(funcName),args_(std::move(args)),retType_(std::move(retType)), body_(std::move(body)),isExternal_(isExternal)
{
}

void Parse::FunctionDecl::print(std::string indent, bool last) {
    std::cout << indent << "+-Function " << funcName_ <<" (";
    for (auto it = args_.begin(); it != args_.end();++it) {
        it->first->print(indent, last);
        std::cout << " "<<it->second;
        if (it != args_.end() - 1) std::cout << ",";
    }
    std::cout << ") -> ";
    retType_->print(indent, last);
    if(isExternal_) {
        std::cout << " external" << std::endl;
    }else {
        std::cout << std::endl;
        body_->print(indent, last);
    }
    //indent += last ? "  " : "| ";

}

Parse::ClassDecl::ClassDecl(const std::string& name): name_(name) {
}

void Parse::ClassDecl::addMemberFunction(std::unique_ptr<FunctionDecl> func) {
    memberFunctions_.emplace_back(std::move(func));
}

void Parse::ClassDecl::addMemberVariable(std::unique_ptr<Stmt> type, const std::string& name) {
    memberVariables_.emplace_back(std::move(type), name);
}

void Parse::ClassDecl::addConstructor(std::unique_ptr<FunctionDecl> func)
{
    constructors_.emplace_back(std::move(func));
}

void Parse::ClassDecl::setDestructor(std::unique_ptr<FunctionDecl> func)
{
    destructor_=std::move(func);
}

void Parse::ClassDecl::print(std::string indent, bool last) {
    std::cout << indent << "+-ClassDecl " << name_ << std::endl;
    indent += last ? "  " : "| ";
    std::cout << indent << "+-member variables" << std::endl;
    auto extraindent = memberFunctions_.empty() ? "  " : "| ";
    for (auto& p : memberVariables_) {
        std::cout << indent << extraindent << "+-" << p.second << " " ;
        p.first->print(indent, last);
        std::cout << std::endl;
    }
    for (size_t i = 0; i < memberFunctions_.size(); ++i) {
        memberFunctions_[i]->print(indent, i == memberFunctions_.size() - 1);
    }
}

void Parse::ClassDecl::toLLVM(ASTContext* context)
{
    std::vector<std::pair<Type*, std::string>> memberList;
    for(auto&p:memberVariables_)
    {
        p.first->toLLVMAST(context);
        auto type = p.first->getType();
        memberList.emplace_back(type, p.second);
    }
    context->addType(name_, std::move(memberList));
    
}

void Parse::FunctionDecl::toLLVM(ASTContext* context)
{
    
}
