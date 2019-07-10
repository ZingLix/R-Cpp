#include "SymbolTable.h"
#include "CodeGenerator.h"

SymbolTable::SymbolTable()
{
    NamedValues.emplace_back();
    NamedFunctions.emplace_back();
}

void SymbolTable::createNewScope()
{
    NamedValues.emplace_back();
    NamedFunctions.emplace_back();
}

void SymbolTable::destroyScope()
{
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
    return is_builtin_type(name);
}