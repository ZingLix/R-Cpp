#include "SymbolTable.h"
#include "../CodeGenerator/CodeGenerator.h"

using namespace Parse;

SymbolTable::SymbolTable():helper_("",nullptr),cur_namespace_(&helper_)
{
    createScope();
}

void SymbolTable::createScope()
{
    named_values_.emplace_back();
    named_values_seq_.emplace_back();
}

void SymbolTable::destroyScope()
{
    named_values_.pop_back();
    named_values_seq_.pop_back();
}

void SymbolTable::createNamespace(const std::string& name)
{
    cur_namespace_->createNewNS(name);
    ns_hierarchy_.push_back(name);
    cur_namespace_ = cur_namespace_->nextNS[name].get();
}

void SymbolTable::destroyNamespace()
{
    ns_hierarchy_.pop_back();
    cur_namespace_ = cur_namespace_->lastNS;
}


Variable* SymbolTable::getVariable(const std::string& name)
{
    for (auto it = named_values_.rbegin(); it != named_values_.rend(); ++it)
    {
        auto t = it->find(name);
        if (t != it->end()) return t->second.get();
    }
    return nullptr;
}

void SymbolTable::addVariable(Type* type, const std::string& name)
{
    named_values_.back()[name] = std::make_unique<Variable>(type,name);
    named_values_seq_.back().push_back(name);
}

CompoundType* SymbolTable::addType(const std::string& name, std::vector<std::pair<Type*, std::string>> memberList)
{
    auto type = std::make_unique<CompoundType>(name, std::move(memberList));
    type->setNamespaceHierarchy(cur_namespace_);
    cur_namespace_->namedType[name] = std::move(type);
    return dynamic_cast<CompoundType*>(cur_namespace_->namedType[name].get());
}

Type* SymbolTable::getType(const std::string& t)
{
    NamespaceHelper* ns = cur_namespace_;
    while (ns!=nullptr)
    {
        auto it = ns->namedType.find(t);
        if (it != ns->namedType.end()) return it->second.get();
        ns = ns->lastNS;
    }
    return nullptr;
}

void SymbolTable::addFunction(const std::string& name, std::vector<std::pair<Type*, std::string>> argList, Type* returnType, bool isExternal)
{
    cur_namespace_->namedFunction[name].push_back(std::make_unique<FunctionType>(name,std::move(argList),returnType,nullptr,isExternal));

}

const std::vector<std::unique_ptr<FunctionType>>* SymbolTable::getFunction(const std::string& name, const std::vector<std::string>& ns_hierarchy)
{
    auto cur = cur_namespace_;
    while (cur!=nullptr)
    {
        auto flist = getRawFunction_(name, ns_hierarchy, cur);
        if (flist->size() != 0) return flist;
        cur = cur->lastNS;
    }
    return nullptr;
}

const std::vector<std::string>& SymbolTable::getNamespaceHierarchy()
{
    return ns_hierarchy_;
}

const std::vector<std::unique_ptr<FunctionType>>* SymbolTable::getRawFunction_(const std::string& name, const std::vector<std::string>& ns_hier,NamespaceHelper* ns)
{
    for(size_t i=0;i<ns_hier.size();++i)
    {
        ns = ns->nextNS[ns_hier[i]].get();
        if (ns == nullptr) return {};
    }
    return &(ns->namedFunction[name]);
}

//void SymbolTable::addClassTemplate(ClassTemplate template_)
//{
//    cur_namespace_->classTemplate[template_.type.typeName] = std::move(template_);
//}
//
//ClassTemplate SymbolTable::getClassTemplate(std::string name)
//{
//    auto cur = cur_namespace_;
//    while (cur != nullptr) {
//        auto t = getClassTemplate_(name, cur);
//        if (t.token.size() != 0) return t;
//        cur = cur->lastNS;
//    }
//    return {};
//}
//
//ClassTemplate SymbolTable::getClassTemplate_(std::string name, NamespaceHelper* ns)
//{
//    return ns->classTemplate[name];
//}


void SymbolTable::setAlias(const std::string& newName, Type* oldType)
{
    cur_namespace_->alias[newName] = oldType;
}

//
//std::string SymbolTable::getMangledClassName(VarType type)
//{
//    auto v = getVarType(type);
//    if (v.typeName != "") type=v;
//    if (is_builtin_type(type.typeName)) return type.typeName;
//    auto name = VarType::mangle(type);
//    auto c = getClass(name);
//    if (c.type.typeName != "") return name;
//    if(type.templateArgs.size()!=0)
//    {
//        auto t = getClassTemplate(type.typeName);
//        //parser_.InstantiateTemplate(type,t);
//        return name;
//    }
//    return "";
//}

//void SymbolTable::callDestructorForCurScope(BlockExprAST* block)
//{
//    for (auto it = named_values_seq_.back().rbegin(); it != named_values_seq_.back().rend(); ++it) {
//        auto& v = named_values_.back()[*it];
//        if (!is_builtin_type(v.type.typeName))
//            block->instructions().push_back(parser_.callDestructor(v));
//    }
//}

//std::vector<std::unique_ptr<ExprAST>> SymbolTable::callDestructor()
//{
//    std::vector<std::unique_ptr<ExprAST>> exprs;
//    auto valmap = named_values_.rbegin();
//    for (auto scope = named_values_seq_.rbegin();
//        scope != named_values_seq_.rend(); ++scope,++valmap)
//    {
//        for (auto it = scope->rbegin(); it != scope->rend(); ++it) {
//            auto& v = (*valmap)[*it];
//            if (!is_builtin_type(v.type.typeName))
//                exprs.push_back(parser_.callDestructor(v));
//        }
//    }
//    return exprs;
//}
//
//void SymbolTable::callNamelessVarDestructor(std::vector<std::unique_ptr<ExprAST>>& exprs)
//{
//    for(auto&v:nameless_values_)
//    {
//        exprs.push_back(parser_.callDestructor(v));
//    }
//    nameless_values_.clear();
//}


SymbolTable::ScopeGuard::ScopeGuard(SymbolTable& st): st_(st), block_(nullptr)
{
    st_.createScope();
}

SymbolTable::ScopeGuard::~ScopeGuard()
{
   // if (block_ != nullptr)
        //st_.callDestructorForCurScope(block_);
    st_.destroyScope();
}

void SymbolTable::ScopeGuard::setBlock(BlockExprAST* block)
{
    block_ = block;
}

SymbolTable::NamespaceGuard::NamespaceGuard(SymbolTable& st, const std::string& name): st_(st)
{
    st_.createNamespace(name);
}

SymbolTable::NamespaceGuard::~NamespaceGuard()
{
    st_.destroyNamespace();
}
