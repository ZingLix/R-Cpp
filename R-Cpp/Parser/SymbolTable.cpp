#include "SymbolTable.h"
#include "../CodeGenerator/CodeGenerator.h"

using namespace Parse;

SymbolTable::SymbolTable(Parser& p):parser_(p), helper_("",nullptr),cur_namespace_(&helper_)
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


Variable SymbolTable::getValue(const std::string& name)
{
    for (auto it = named_values_.rbegin(); it != named_values_.rend(); ++it)
    {
        auto t = it->find(name);
        if (t != it->end()) return t->second;
    }
    return {};
}

void SymbolTable::setValue(const std::string& name, Variable val)
{
    named_values_.back()[name] = val;
    named_values_seq_.back().push_back(name);
}

bool SymbolTable::hasType(const std::string& t) {
    if(!is_builtin_type(t))
        return getClass(t).type.typeName != ""||getClassTemplate(t).type.typeName!="";
    return true;
}

void SymbolTable::addClass(const std::string& name, Class c)
{
    c.type.namespaceHierarchy = ns_hierarchy_;
    cur_namespace_->namedClass[name] = std::move(c);
}

Class SymbolTable::getClass(const std::string& t)
{
    NamespaceHelper* ns = cur_namespace_;
    while (ns!=nullptr)
    {
        auto it = ns->namedClass.find(t);
        if (it != ns->namedClass.end()) return it->second;
        ns = ns->lastNS;
    }
    return {};
}

VarType SymbolTable::getClassMemberType(const std::string& classType, const std::string& memberName)
{
    auto c = getClass(classType);
    for(auto& v:c.memberVariables)
    {
        if (v.name == memberName) return v.type;
    }
    return VarType("");
}

int SymbolTable::getClassMemberIndex(const std::string& className, const std::string& memberName)
{
    auto c = getClass(className);
    for(auto it=c.memberVariables.begin();it!=c.memberVariables.end();++it)
    {
        if (it->name == memberName) return it - c.memberVariables.begin();
    }
    return -1;
}

void SymbolTable::addFunction(const ::Function& func)
{
    cur_namespace_->namedFunction[func.name].push_back(func);
}

const std::vector<Function>* SymbolTable::getFunction(const std::string& name, const std::vector<std::string>& ns_hierarchy)
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

const std::vector<Function>* SymbolTable::getRawFunction_(const std::string& name, const std::vector<std::string>& ns_hier,NamespaceHelper* ns)
{
    for(size_t i=0;i<ns_hier.size();++i)
    {
        ns = ns->nextNS[ns_hier[i]].get();
        if (ns == nullptr) return {};
    }
    return &(ns->namedFunction[name]);
}

void SymbolTable::addClassTemplate(ClassTemplate template_)
{
    cur_namespace_->classTemplate[template_.type.typeName] = std::move(template_);
}

ClassTemplate SymbolTable::getClassTemplate(std::string name)
{
    auto cur = cur_namespace_;
    while (cur != nullptr) {
        auto t = getClassTemplate_(name, cur);
        if (t.token.size() != 0) return t;
        cur = cur->lastNS;
    }
    return {};
}

ClassTemplate SymbolTable::getClassTemplate_(std::string name, NamespaceHelper* ns)
{
    return ns->classTemplate[name];
}

VarType SymbolTable::getVarType(VarType type)
{
    if (is_builtin_type(type.typeName)) return type;
    auto cur = cur_namespace_;
    while (cur != nullptr) {
        auto v = cur->alias[type];
        if (v.typeName != "") return v;
        cur = cur->lastNS;
    }
    return type;
}

void SymbolTable::setAlias(VarType newType, VarType oldType)
{
    cur_namespace_->alias[newType] = oldType;
}


std::string SymbolTable::getMangledClassName(VarType type)
{
    auto v = getVarType(type);
    if (v.typeName != "") type=v;
    if (is_builtin_type(type.typeName)) return type.typeName;
    auto name = VarType::mangle(type);
    auto c = getClass(name);
    if (c.type.typeName != "") return name;
    if(type.templateArgs.size()!=0)
    {
        auto t = getClassTemplate(type.typeName);
        parser_.InstantiateTemplate(type,t);
        return name;
    }
    return "";
}

void SymbolTable::callDestructorForCurScope(BlockExprAST* block)
{
    for (auto it = named_values_seq_.back().rbegin(); it != named_values_seq_.back().rend(); ++it) {
        auto& v = named_values_.back()[*it];
        if (!is_builtin_type(v.type.typeName))
            block->instructions().push_back(parser_.callDestructor(v));
    }
}

std::vector<std::unique_ptr<ExprAST>> SymbolTable::callDestructor()
{
    std::vector<std::unique_ptr<ExprAST>> exprs;
    auto valmap = named_values_.rbegin();
    for (auto scope = named_values_seq_.rbegin();
        scope != named_values_seq_.rend(); ++scope,++valmap)
    {
        for (auto it = scope->rbegin(); it != scope->rend(); ++it) {
            auto& v = (*valmap)[*it];
            if (!is_builtin_type(v.type.typeName))
                exprs.push_back(parser_.callDestructor(v));
        }
    }
    return exprs;
}

void SymbolTable::callNamelessVarDestructor(std::vector<std::unique_ptr<ExprAST>>& exprs)
{
    for(auto&v:nameless_values_)
    {
        exprs.push_back(parser_.callDestructor(v));
    }
    nameless_values_.clear();
}


SymbolTable::ScopeGuard::ScopeGuard(SymbolTable& st): st_(st), block_(nullptr)
{
    st_.createScope();
}

SymbolTable::ScopeGuard::~ScopeGuard()
{
    if (block_ != nullptr)
        st_.callDestructorForCurScope(block_);
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
