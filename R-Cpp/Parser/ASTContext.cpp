#include "ASTContext.h"
#include "SymbolTable.h"
#include "AST.h"

Parse::ASTContext::ASTContext(): symbol_table_(std::make_unique<SymbolTable>(*this)), nameless_var_count_(1),
                                 cur_parsing_class_(nullptr) {

}

Parse::SymbolTable& Parse::ASTContext::symbolTable() {
    return *symbol_table_;
}

int64_t Parse::ASTContext::getNamelessVarCount() {
    return nameless_var_count_++;
}

void Parse::ASTContext::addClassAST(std::unique_ptr<ClassAST> ast) {
    assert(ast != nullptr);
    classes_.push_back(std::move(ast));
}

void Parse::ASTContext::addClassTemplate(std::vector<std::pair<std::string, std::string>> arglist, std::unique_ptr<ClassDecl> decl)
{
    symbolTable().addClassTemplate(std::move(arglist), std::move(decl));
}

Parse::Type* Parse::ASTContext::
addType(const std::string& name, std::vector<std::pair<Type*, std::string>> memberList) {
    auto t = symbol_table_->addType(name, std::move(memberList));
    addLLVMType(t);
    return t;
}

Parse::FunctionType* Parse::ASTContext::addFuncPrototype(const std::string& name,
                                                         std::vector<std::pair<Type*, std::string>> argList,
                                                         Type* returnType, bool isExternal) {
    std::vector<std::pair<std::string, std::string>> members;
    for (auto& m : argList) {
        members.emplace_back(m.first->mangledName(), m.second);
    }
    auto fn = symbol_table_->addFunction(name, std::move(argList), returnType,currentClass(),  isExternal);
    prototype_.push_back(std::make_unique<PrototypeAST>(fn->mangledName(), std::move(members),
                                                        returnType->mangledName(),
                                                        currentClass() == nullptr
                                                            ? ""
                                                            : currentClass()->mangledName()));
    return fn;
}

void Parse::ASTContext::setFuncBody(FunctionType* func, std::unique_ptr<BlockExprAST> body) {
    auto c = currentClass();
    functions_.push_back(
        std::make_unique<FunctionAST>(func->mangledName(), std::move(body), c == nullptr ? "" : c->mangledName()));

}

Parse::LiteralType* Parse::ASTContext::addLiteralType(LiteralType::category type, int64_t val)
{
    literal_types_.push_back(std::make_unique<LiteralType>(LiteralType::category::Integer, val));
    return literal_types_.back().get();
}

std::vector<std::unique_ptr<ClassAST>>* Parse::ASTContext::Class() {
    return &classes_;
}

std::vector<std::unique_ptr<PrototypeAST>>* Parse::ASTContext::Prototype() {
    return &prototype_;
}

std::vector<std::unique_ptr<FunctionAST>>* Parse::ASTContext::Function() {
    return &functions_;
}

Parse::CompoundType* Parse::ASTContext::currentClass() const {
    return cur_parsing_class_;
}

void Parse::ASTContext::setCurrentClass(CompoundType* t) {
    cur_parsing_class_ = t;
    symbolTable().createScope();
    for (auto arg : cur_parsing_class_->getMemberVariables()) {
        symbolTable().addVariable(arg.first, arg.second);
    }
}

void Parse::ASTContext::unsetCurrentClass() {
    cur_parsing_class_ = nullptr;
    symbolTable().destroyScope();
}

std::string Parse::ASTContext::namelessVarName() {
    return "__" + std::to_string(nameless_var_count_++);
}

void Parse::ASTContext::addLLVMType(CompoundType* t)
{
    std::vector<std::pair<std::string, std::string>> members;
    for (auto& m : t->getMemberVariables()) {
        members.emplace_back(m.first->mangledName(), m.second);
    }
    classes_.push_back(std::make_unique<ClassAST>(t->mangledName(), std::move(members), t));
}
