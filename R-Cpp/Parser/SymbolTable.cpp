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
}

void SymbolTable::destroyScope()
{
    named_values_.pop_back();
}

void SymbolTable::createNamespace(const std::string& name)
{
    helper_.createNewNS(name);
    ns_hierarchy_.push_back(name);
    cur_namespace_ = helper_.nextNS[name].get();
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
    return Variable();
}

void SymbolTable::setValue(const std::string& name, Variable val)
{
    named_values_.back()[name] = val;
}

bool SymbolTable::hasType(const VarType& t) {
    if(!is_builtin_type(t.typeName))
        return getClass(t).type.typeName != "";
    return true;
}

void SymbolTable::addClass(const VarType& name, Class c)
{
    c.type.namespaceHierarchy = ns_hierarchy_;
    cur_namespace_->namedClass[name] = std::move(c);
}

Class SymbolTable::getClass(const VarType& t)
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

VarType SymbolTable::getClassMemberType(const VarType& classType, const std::string& memberName)
{
    auto c = getClass(classType);
    for(auto& v:c.memberVariables)
    {
        if (v.name == memberName) return v.type;
    }
    return VarType("");
}

int SymbolTable::getClassMemberIndex(const VarType& className, const std::string& memberName)
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

const std::vector<std::string>& SymbolTable::getNamespaceHierachy()
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
    cur_namespace_->classTemplate[template_.type] = std::move(template_);
}

ClassTemplate SymbolTable::getClassTemplate(VarType name)
{
    auto cur = cur_namespace_;
    while (cur != nullptr) {
        auto t = getClassTemplate_(name, cur);
        if (t.token.size() != 0) return t;
        cur = cur->lastNS;
    }
    return {};
}

ClassTemplate SymbolTable::getClassTemplate_(VarType name, NamespaceHelper* ns)
{
    return ns->classTemplate[name];
}
