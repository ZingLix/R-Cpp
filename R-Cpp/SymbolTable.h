#pragma once
#include "Type.h"
#include "Operator.h"

class SymbolTable
{
public:
    SymbolTable();

    void createNewScope();

    void destroyScope();

    Variable getValue(const std::string& name);

    void setValue(const std::string& name, Variable val);

    void addFunction(const std::string& name, ::Function func);

    ::Function getFunction(const std::string& name);

    bool hasType(const std::string& name);
    void addClass(const std::string& name, Class c);
    Class getClass(const std::string& name);
    llvm::Type* getType(const std::string& name);
    void setType(const std::string& name, llvm::Type* t);
    std::string getClassMemberType(const std::string& className, const std::string& memberName);
    int getClassMemberIndex(const std::string& className, const std::string& memberName);

private:
    std::vector<std::map<std::string, Variable>> NamedValues;
    std::vector<std::map<std::string, ::Function>> NamedFunctions;
    std::vector<std::map<std::string, Class>> NamedClass;
//    std::map < std::pair<std::string, OperatorType>, std::map<std::string, std::function<
//        llvm::Value* (llvm::Value*, llvm::Value*, CodeGenerator&)>>> OperatorMap;
//    std::map < std::string, std::map<OperatorType,
//        std::map<std::string, std::function<
//            llvm::Value* (llvm::Value*, llvm::Value*,CodeGenerator&)>>>> OperatorMap;
};



class ScopeGuard
{
public:
    ScopeGuard(SymbolTable& st)
        :st_(st) {
        st_.createNewScope();
    }

    ~ScopeGuard() {
        st_.destroyScope();
    }

private:
    SymbolTable& st_;
};
