#pragma once
#include "Type.h"
#include "Operator.h"

struct NamespaceHelper
{
    NamespaceHelper(const std::string& nsName,NamespaceHelper* last)
        :name(nsName),lastNS(last){ }

    void createNewNS(const std::string& name)
    {
        nextNS[name] = std::make_unique<NamespaceHelper>(name, this);
    }

    std::string name;
    std::map<std::string, std::unique_ptr<NamespaceHelper>> nextNS;
    std::vector<NamespaceHelper*> insertedNS;
    std::map<std::string, std::vector<Function>> namedFunction;
    std::map<VarType, Class> namedClass;
    NamespaceHelper* lastNS;
};

class SymbolTable
{
public:
    SymbolTable();
    void createScope();
    void destroyScope();
    void createNamespace(const std::string& name);
    void destroyNamespace();

    Variable getValue(const std::string& name);
    void setValue(const std::string& name, Variable val);
    void addFunction(const std::string& name, Function func);
    Function getMangledFunction(const std::string& name);
    bool hasType(const VarType& name);
    void addClass(const VarType& name, Class c);
    Class getClass(const VarType& name);
    llvm::Type* getLLVMType(const VarType& type);
    void setLLVMType(const VarType& type, llvm::Type* t);
    VarType getClassMemberType(const VarType& classType, const std::string& memberName);
    int getClassMemberIndex(const VarType& classType, const std::string& memberName);
    void addRawFunction(const ::Function& func);
    const std::vector<::Function>* getRawFunction(const std::string& name, const std::vector<std::string>& ns_hierarchy={});
    const std::vector<std::string>& getNamespaceHierachy();

    class ScopeGuard
    {
    public:
        ScopeGuard(SymbolTable& st)
            :st_(st) {
            st_.createScope();
        }

        ~ScopeGuard() {
            st_.destroyScope();
        }

    private:
        SymbolTable& st_;
    };
    class NamespaceGuard
    {
    public:
        NamespaceGuard(SymbolTable& st,const std::string& name)
            :st_(st) {
            st_.createNamespace(name);
        }

        ~NamespaceGuard() {
            st_.destroyNamespace();
        }

    private:
        SymbolTable& st_;
    };
private:
    const std::vector<Function>* getRawFunction_(const std::string& name, const std::vector<std::string>& ns_hierarchy, NamespaceHelper* ns);


    std::vector<std::map<std::string, Variable>> named_values_;
    NamespaceHelper helper_;
    NamespaceHelper* cur_namespace_;
    std::vector<std::string> ns_hierarchy_;
    std::map<std::string, Function> mangled_function_map_;
};


