#include "AST.h"
#include <iostream>

std::string toXMLPair(const std::string& tag,const std::string& content) {
    return "<" + tag + ">" + content + "</" + tag + ">";
}

Parse::FunctionType* findSuitableFunction(const std::vector<std::unique_ptr<Parse::Stmt>>& argList, const std::vector<Parse::FunctionType*>* fnList) {
    Parse::FunctionType* target = nullptr;
    for (auto& f : *fnList) {
        bool flag = true;
        if (f->args().size() == argList.size()) {
            size_t i = 0;
            while (i < f->args().size()) {
                if (f->args()[i].first != argList[i]->getType()) {
                    flag = false;
                    break;
                }
                ++i;
            }
            if (flag) target = f;
        }
    }
    if (!target) {
        throw std::logic_error("No suitable function.");
    }
    return target;
}

Parse::FunctionType* findSuitableFunction(const std::vector<std::unique_ptr<Parse::Stmt>>& argList, const std::vector<std::unique_ptr<Parse::FunctionType>>* fnList) {
    Parse::FunctionType* target = nullptr;
    for (auto& f : *fnList) {
        bool flag = true;
        if (f->args().size() == argList.size()) {
            size_t i = 0;
            while (i < f->args().size()) {
                if (f->args()[i].first != argList[i]->getType()) {
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
    return target;
}

Parse::CompoundStmt::CompoundStmt(std::vector<std::unique_ptr<Stmt>> exprs)
    : stmts_(std::move(exprs)) 
{ }

void Parse::CompoundStmt::print(std::string indent, bool last) {
    indent += last ? "  " : "| ";
    for (size_t i = 0; i < stmts_.size(); ++i) {
        stmts_[i]->print(indent, i == stmts_.size() - 1);
    }
}

std::string Parse::CompoundStmt::dumpToXML() const {
    std::string ret;
    for(auto& stmt:stmts_) {
        ret += stmt->dumpToXML();
    }
    return ret;
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

std::string Parse::IfStmt::dumpToXML() const {
    std::string str = "<IfStmt>";
    str += toXMLPair("condition", cond_->dumpToXML());
    str += toXMLPair("then", then_->dumpToXML());
    if (else_) str += toXMLPair("then", else_->dumpToXML());
    str += "</IfStmt>";
    return str;
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

std::string Parse::ForStmt::dumpToXML() const {
    std::string str = "<ForStmt>";
    str += toXMLPair("start", start_->dumpToXML());
    str += toXMLPair("condition", cond_->dumpToXML());
    str += toXMLPair("end", end_->dumpToXML());
    str += toXMLPair("body", body_->dumpToXML());
    str += "</ForStmt>";
    return str;
}

std::unique_ptr<ExprAST> Parse::ForStmt::toLLVMAST(ASTContext* context)
{
    auto start = start_->toLLVMAST(context);
    auto cond = cond_->toLLVMAST(context);
    auto end = end_->toLLVMAST(context);
    auto body = body_->toBlockExprAST(context);
    return std::make_unique<ForExprAST>(std::move(start), std::move(cond),
        end_->toLLVMAST(context), std::move(body));
}

Parse::ReturnStmt::ReturnStmt(std::unique_ptr<Stmt> returnVal): ret_val_(std::move(returnVal)) {
}

void Parse::ReturnStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-ReturnStmt" << std::endl;
    indent += last ? "  " : " |";
    ret_val_->print(indent, true);
}

std::string Parse::ReturnStmt::dumpToXML() const {
    return toXMLPair("ReturnStmt", ret_val_->dumpToXML());
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

std::string Parse::BinaryOperatorStmt::dumpToXML() const {
    std::string str = "<BinaryOperatorStmt operator=\"" + operatorDescription(op_) + "\">";
    str += toXMLPair("LHS", lhs_->dumpToXML());
    str += toXMLPair("RHS", rhs_->dumpToXML());
    str += "</BinaryOperatorStmt>";
    return str;
}

std::unique_ptr<ExprAST> Parse::BinaryOperatorStmt::toLLVMAST(ASTContext* context)
{
    if(op_==OperatorType::ScopeResolution) {
        auto l = dynamic_cast<VariableStmt*>(lhs_.get());
        if (!l) throw std::logic_error("Left hand of :: is not a valid scope name.");
        auto ns = context->symbolTable().getNamespace(l->getName());
        if (!ns) throw std::logic_error("No namespace named " + l->getName() + ".");
        context->symbolTable().setSpecfiedNamespace(ns);
        auto ret= rhs_->toLLVMAST(context);
        context->symbolTable().unsetSpecfiedNamespace();
        return ret;
    }
    auto l = lhs_->toLLVMAST(context);
    if(op_==OperatorType::MemberAccessP||op_==OperatorType::MemberAccessA)
    {
        auto t = lhs_->getType();
        if(op_==OperatorType::MemberAccessA) {
            if (t->getTypename() != "__ptr") throw std::logic_error("Operator -> only suits for pointer.");
            t = t->getTemplateArgs()[0];
            lhs_->setType(t);
            l = std::make_unique<UnaryExprAST>(std::move(l), OperatorType::Dereference, t->mangledName());
        }
        auto type = dynamic_cast<CompoundType*>(t);
        if(!type)
        {
            throw std::logic_error("Invalid member access.");
        }
        auto r = dynamic_cast<VariableStmt*>(rhs_.get());
        auto index = type->getMemberIndex(r->getName());
        if(index!=-1) {
            type_ = type->getMemberType(r->getName());
            return std::make_unique<MemberAccessAST>(std::move(l), index, type_->mangledName());
        }
        auto funclist = type->getFunction(r->getName());
        if(funclist==nullptr) {
            throw std::logic_error("Unknown member.");
        }
        type_ = (*funclist)[0];
        auto ret = std::make_unique<CallExprAST>(r->getName(), std::vector<std::unique_ptr<ExprAST>>{}, "null");
        ret->setThis(std::move(l));
        return ret;
    }
    auto r = rhs_->toLLVMAST(context);
    if(lhs_->getType()!=rhs_->getType())
    {
        throw std::logic_error("No suitable binary operator between "+lhs_->getType()->getTypename()+" and "+rhs_->getType()->getTypename()+".");
    }
    if(op_==OperatorType::Assignment)
    {
        type_ = context->symbolTable().getType("void");
    }else
    {
        type_ = lhs_->getType();
    }
    return std::make_unique<BinaryExprAST>(op_, std::move(l), std::move(r),l->getType());
}

Parse::Type* Parse::BinaryOperatorStmt::getLHSType() {
    return lhs_->getType();
}

Parse::Type* Parse::BinaryOperatorStmt::getRHSType() {
    return rhs_->getType();
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

std::string Parse::UnaryOperatorStmt::dumpToXML() const {
    std::string str = "<UnaryOperatorStmt operator=\"" + operatorDescription(op_) + "\">";
    str += toXMLPair("operand",stmt_->dumpToXML());
    for(auto& arg:args_) {
        str += toXMLPair("arugment", arg->dumpToXML());
    }
    str += "</UnaryOperatorStmt>";
    return str;

}

std::unique_ptr<ExprAST> Parse::UnaryOperatorStmt::toLLVMAST(ASTContext* context) {
    auto expr = stmt_->toLLVMAST(context);
    std::vector<std::unique_ptr<ExprAST>> argsExpr;
    for(auto&arg:args_)
    {
        argsExpr.push_back(arg->toLLVMAST(context));
    }
    auto fn = dynamic_cast<FunctionType*>(stmt_->getType());
    if(fn &&op_==OperatorType::FunctionCall) {
        auto call = dynamic_cast<CallExprAST*>(expr.get());
        if(call) {
            // a.fun()
            auto stmt = dynamic_cast<BinaryOperatorStmt*>(stmt_.get());
            auto fnList = dynamic_cast<CompoundType*>(stmt->getLHSType())->getFunction(call->getName());
            auto target = findSuitableFunction(args_, fnList);
            type_ = target->returnType();
            call->setType(type_->mangledName());
            call->setName(target->mangledName());
            call->setArgs(std::move(argsExpr));
            return expr;
        }else {
            // func()
            auto fnList = context->symbolTable().getFunction(fn->getTypename());
            auto target = findSuitableFunction(args_, fnList);
            type_ = target->returnType();
            return std::make_unique<CallExprAST>(target->mangledName(), std::move(argsExpr), target->returnType()->mangledName());

        }
    }
    auto type = dynamic_cast<TypeStmt*>(stmt_.get());
    if(type && op_==OperatorType::FunctionCall) {
        auto fnlist = dynamic_cast<CompoundType*>(type->getType())->getConstructors();
        auto target = findSuitableFunction(args_, fnlist);
        type_ = type->getType();
        auto name = context->namelessVarName();
        context->symbolTable().addNamelessVariable(type_, name);
        return std::make_unique<NamelessVarExprAST>(name, type_->mangledName(),target->mangledName(),std::move(argsExpr));
    }
    auto var = dynamic_cast<VariableStmt*>(stmt_.get());
    if(var && op_==OperatorType::Subscript)
    {
        if(var->getType()->getTypename()!="__arr")
        {
            throw std::logic_error("No suitable operation between " + var->getType()->getTypename() + " and [].");
        }
        type_ = var->getType()->getTemplateArgs()[0];
        return std::make_unique<UnaryExprAST>(std::move(expr), OperatorType::Subscript, std::move(argsExpr), type_->mangledName());
    }
    throw std::logic_error("No suitable unary operation.");
}

Parse::VariableDefStmt::VariableDefStmt(std::unique_ptr<Stmt> type, const std::string& name)
    : vartype_(std::move(type)), name_(name)
{ }

void Parse::VariableDefStmt::setInitValue(std::unique_ptr<Stmt> initVal) {
    init_val_ = std::move(initVal);
}

void Parse::VariableDefStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-VariableDefStmt " << name_ << " ";
    std::cout << dynamic_cast<TypeStmt*>(vartype_.get())->getName();
    std::cout << std::endl;
    indent += last ? "  " : "| ";
    if (init_val_)
        init_val_->print(indent, true);
}

std::string Parse::VariableDefStmt::dumpToXML() const {
    std::string str = "<VariableDefStmt name=\"" + name_ + "\">";
    str += toXMLPair("type", vartype_->dumpToXML());
    if(init_val_)
        str += toXMLPair("initVal", init_val_->dumpToXML());
    str += "</VariableDefStmt>";
    return str;
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

Parse::TypeStmt::
TypeStmt(const std::string& name, std::vector<std::unique_ptr<Stmt>> arglist): name_(name), arglist_(std::move(arglist))
{
}

void Parse::TypeStmt::print(std::string indent, bool last)
{
    std::cout << indent << "+-Type/Function " << getName() << std::endl;
}

std::string Parse::TypeStmt::getName()
{
    std::string name = name_;
    if (arglist_.size() != 0)
    {
        name += "<";
        for (size_t i = 0; i < arglist_.size(); ++i)
        {
            auto type = dynamic_cast<TypeStmt*>(arglist_[i].get());
            if(type)
                name += type->getName();
            else
            {
                auto num = dynamic_cast<IntegerStmt*>(arglist_[i].get());
                assert(num != nullptr);
                name += std::to_string(num->getNumber());
            }
            if (i != arglist_.size() - 1) name += ", ";
        }
        name += ">";
    }
    return name;
}

std::string Parse::TypeStmt::dumpToXML() const {
    std::string str = "<TypeStmt name=\"" + name_ + "\">";
    for(auto& arg:arglist_) {
        str += toXMLPair("argument", arg->dumpToXML());
    }
    str += "</TypeStmt>";
    return str;
}

std::unique_ptr<ExprAST> Parse::TypeStmt::toLLVMAST(ASTContext* context)
{   // do not return ExprAST, only to check whether the type is valid.
    // Return nullptr when everything is ok, otherwise an exception will
    // be thrown out.
    std::vector<Type*> typelist;
    for(auto& stmt:arglist_)
    {
        stmt->toLLVMAST(context);
        if(dynamic_cast<TypeStmt*>(stmt.get()))
            typelist.push_back(stmt->getType());
        else 
        {
            auto integer = dynamic_cast<IntegerStmt*>(stmt.get());
            if (integer)
                typelist.push_back(context->addLiteralType(LiteralType::category::Integer, integer->getNumber()));
            else
                throw std::logic_error("Unsupported type in template args.");
        }
    }
    type_ = context->symbolTable().getType(name_, typelist);
    if(!type_)
    {
        throw std::logic_error("Unknown type.");
    }
    return nullptr;
}

Parse::VariableStmt::VariableStmt(const std::string& name): name_(name) {
}

void Parse::VariableStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-VariableStmt " << name_ << std::endl;
}

std::string Parse::VariableStmt::dumpToXML() const {
    return "<VariableStmt name=\"" + name_ + "\"></VariableStmt>";
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

const std::string& Parse::VariableStmt::getName()
{
    return name_;
}

Parse::IntegerStmt::IntegerStmt(std::int64_t val): val_(val) {
}

void Parse::IntegerStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-IntegerStmt " << val_ << std::endl;
}

std::int64_t Parse::IntegerStmt::getNumber() const
{
    return val_;
}

std::string Parse::IntegerStmt::dumpToXML() const {
    return "<IntegerStmt value=" + std::to_string(val_) + "></IntegerStmt>";
}

std::unique_ptr<ExprAST> Parse::IntegerStmt::toLLVMAST(ASTContext*c)
{
    type_ = c->symbolTable().getType("i32");
    return std::make_unique<IntegerExprAST>(val_);
}

Parse::FloatStmt::FloatStmt(double val): val_(val) {
}

void Parse::FloatStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-FloatStmt " << val_ << std::endl;
}

double Parse::FloatStmt::getNumber() const {
    return val_;
}

std::string Parse::FloatStmt::dumpToXML() const {
    return "<FloatStmt value=" + std::to_string(val_) + "></FloatStmt>";
}

std::unique_ptr<ExprAST> Parse::FloatStmt::toLLVMAST(ASTContext* c)
{
    type_ = c->symbolTable().getType("float");
    return std::make_unique<FloatExprAST>(val_);
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
        std::cout << dynamic_cast<TypeStmt*>(it->first.get())->getName();
        std::cout << " "<<it->second;
        if (it != args_.end() - 1) std::cout << ",";
    }
    std::cout << ") -> ";
    std::cout << dynamic_cast<TypeStmt*>(retType_.get())->getName();
    if(isExternal_) {
        std::cout << " external" << std::endl;
    }else {
        std::cout << std::endl;
        body_->print(indent, last);
    }
    //indent += last ? "  " : "| ";

}

void Parse::FunctionDecl::setBody(std::unique_ptr<CompoundStmt> body)
{
    body_ = std::move(body);
}

std::string Parse::FunctionDecl::dumpToXML() const {
    std::string str = "<FunctionDecl name=\"" + funcName_ + "\" external=\"" + (isExternal_ ? "true" : "false") + "\">";
    str += "<arguments>";
    for(auto& arg:args_) {
        str += "<argument name=\""+arg.second+"\">";
        str+=arg.first->dumpToXML();
        str += "</argument>";
    }
    str += "</arguments>";
    str += toXMLPair("returnType", retType_->dumpToXML());
    if(body_)
        str += toXMLPair("body", body_->dumpToXML());
    str += "</FunctionDecl>";
    return str;
}

Parse::ClassDecl::ClassDecl(const std::string& name): name_(name),classType_(nullptr) {
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
        std::cout << dynamic_cast<TypeStmt*>(p.first.get())->getName();
        std::cout << std::endl;
    }
    for (size_t i = 0; i < memberFunctions_.size(); ++i) {
        memberFunctions_[i]->print(indent, i == memberFunctions_.size() - 1);
    }
}

std::string Parse::ClassDecl::dumpToXML() const {
    std::string str ="<ClassDecl name=\""+name_+"\">";
    str += "<memberVariables>";
    for(auto& var:memberVariables_) {
        str += "<variable name=\"" + var.second + "\">";
        str += var.first->dumpToXML();
        str += "</variable>";
    }
    str += "</memberVariables>";
    str += "<constructors>";
    for (auto& var : constructors_) {
        str += toXMLPair("constructor", var->dumpToXML());
    }
    str += "</constructors>";
    if(destructor_)
        str += toXMLPair("desturctor", destructor_->dumpToXML());
    str += "<memberFunctions>";
    for (auto& var : memberFunctions_) {
        str += var->dumpToXML();
    }
    str += "</memberFunctions>";
    str += "</ClassDecl>";
    return str;
}

void Parse::ClassDecl::toLLVM(ASTContext* context)
{
    std::vector<std::pair<Type*, std::string>> memberList;
    for(auto& p:memberVariables_)
    {
        p.first->toLLVMAST(context);
        auto type = p.first->getType();
        memberList.emplace_back(type, p.second);
    }
    classType_ = dynamic_cast<CompoundType*>(context->addType(name_, std::move(memberList)));
    generateNewFunction(context);
}

void Parse::ClassDecl::generateNewFunction(ASTContext* context) {
    std::vector<Type*> typeArgs;
    typeArgs.push_back(classType_);
    SymbolTable::NamespaceGuard guard(context->symbolTable(), name());
    context->symbolTable().addFunction(
    "new", std::vector<std::pair<Type*, std::string>>{},context->symbolTable().getType("__ptr",typeArgs));
}

const std::vector<std::pair<std::unique_ptr<Parse::Stmt>, std::string>>& Parse::ClassDecl::getMemberVariables()
{
    return memberVariables_;
}

void Parse::FunctionDecl::toLLVM(ASTContext* context)
{
    SymbolTable::ScopeGuard guard(context->symbolTable());
    for (auto& arg : funcType_->args()) {
        context->symbolTable().addVariable(arg.first, arg.second);
    }
    if (!isExternal_&&body_) {
        auto body = body_->toBlockExprAST(context);
        context->setFuncBody(funcType_, std::move(body));
    }
}

Parse::FunctionType* Parse::FunctionDecl::registerPrototype(ASTContext* context) {
    std::vector<std::pair<Type*, std::string>> arglist;
    auto curClass = context->currentClass();
    for (auto& p : args_) {
        p.first->toLLVMAST(context);
        auto type = p.first->getType();
        arglist.emplace_back(type, p.second);
    }
    retType_->toLLVMAST(context);
    funcType_ = context->addFuncPrototype(funcName_, std::move(arglist), retType_->getType(), isExternal_);
    
    return funcType_;
}

void Parse::ClassDecl::registerMemberFunction(ASTContext* context) {
    ASTContext::ClassScopeGuard guard(*context, classType_);
    for(auto& f:constructors_) {
        
        classType_->addConstructor(f->registerPrototype(context));
    }
    for(auto& f:memberFunctions_) {
        classType_->addFunction(f->registerPrototype(context));
    }
    if(destructor_)
        classType_->setDestructor(destructor_->registerPrototype(context));
    for (auto& f : constructors_) {
        f->toLLVM(context);
    }
    for (auto& f : memberFunctions_) {
        f->toLLVM(context);
    }
    if (destructor_)
        destructor_->toLLVM(context);
}

