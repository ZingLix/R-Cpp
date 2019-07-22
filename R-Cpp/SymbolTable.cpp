#include "SymbolTable.h"
#include "CodeGenerator.h"

SymbolTable::SymbolTable()
{
    createNewScope();
}

void SymbolTable::createNewScope()
{
    NamedValues.emplace_back();
    NamedFunctions.emplace_back();
    NamedClass.emplace_back();
}

void SymbolTable::destroyScope()
{
    NamedClass.pop_back();
    NamedFunctions.pop_back();
    NamedValues.pop_back();
}

Variable SymbolTable::getValue(const std::string& name)
{
    for (auto it = NamedValues.rbegin(); it != NamedValues.rend(); ++it)
    {
        auto t = it->find(name);
        if (t != it->end()) return t->second;
    }
    return Variable();
}

void SymbolTable::setValue(const std::string& name, Variable val)
{
    NamedValues.back()[name] = val;
}

void SymbolTable::addFunction(const std::string& name, ::Function func)
{
    NamedFunctions.back()[name] = func;
}

::Function SymbolTable::getFunction(const std::string& name)
{
    for (auto it = NamedFunctions.rbegin(); it != NamedFunctions.rend(); ++it)
    {
        auto t = it->find(name);
        if (t != it->end()) return t->second;
    }
    return ::Function();
}

bool SymbolTable::hasType(const std::string& name) {
    if(!is_builtin_type(name))
    {
        return getClass(name).name!="";
    }
    return true;
}

void SymbolTable::addClass(const std::string& name, Class c)
{
    NamedClass.back()[name] = c;
}

Class SymbolTable::getClass(const VarType& type)
{
    for(auto it=NamedClass.rbegin();it!=NamedClass.rend();++it)
    {
        auto t = it->find(type.typeName);
        if (t != it->end()) return t->second;
    }
    return Class("");
}


llvm::Type* SymbolTable::getType(VarType type)
{
    if (is_builtin_type(type.typeName)) return nullptr;
    return getClass(type.typeName).type;
}

void SymbolTable::setType(VarType type, llvm::Type* tp)
{
    for (auto it = NamedClass.rbegin(); it != NamedClass.rend(); ++it) {
        auto t = it->find(type.typeName);
        if (t != it->end()) (t->second).type=tp;
    }
}

VarType SymbolTable::getClassMemberType(const VarType& className, const std::string& memberName)
{
    auto c = getClass(className);
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

