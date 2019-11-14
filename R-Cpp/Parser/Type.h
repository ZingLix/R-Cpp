#pragma once
#include <string>
#include <set>
#include <vector>
#include <memory>
#include <map>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace CG
{
    class CodeGenerator;
}

namespace Parse
{
    struct NamespaceHelper;
    class Type
    {
    public:
        const std::string getTypename() const;
        virtual std::string mangledName() = 0;
        virtual ~Type(){}

        Type(const std::string& name, std::vector<Type*> typelist = {});

        void setNamespaceHierarchy(NamespaceHelper* hierarchy);
        NamespaceHelper* getNamespaceHierarchy() const;

    protected:
        std::string name_;
        std::vector<Type*> typelist_;
        NamespaceHelper* namespaceHierarchy_;
    };

    class BuiltinType : public Type
    {
    public:
        BuiltinType(const std::string& typeName, std::vector<Type*> typelist = {});

        std::string mangledName() override;
        static const std::set<std::string>& builtinTypeSet();

    private:
        static const std::set<std::string> builtinTypeSet_;
    };

    class FunctionType : public Type
    {
    public:
        FunctionType(const std::string& functionName, std::vector<std::pair<Type*, std::string>> argTypeList,
                     Type* returnType, Type* classType, bool isExternal = false, std::vector<Type*> typelist = {});

        std::string mangledName() override;
        const std::vector<std::pair<Type*, std::string>>& args();
        Type* returnType();
        Type* classType() const;

    private:
        std::vector<std::pair<Type*,std::string>> argTypeList_;
        Type* returnType_;
        Type* classType_;
        bool isExternal_;
    };

    class CompoundType: public Type
    {
    public:
        CompoundType(const std::string& typeName, std::vector<std::pair<Type*, std::string>> memberList,
                     std::vector<Type*> typelist = {});

        std::string mangledName() override;
        Type* getMemberType(const std::string& name);
        int getMemberIndex(const std::string& name);
        const std::vector<FunctionType*>* getConstructors() const;
        FunctionType* getDestructor() const;
        const std::vector<std::pair<Type*, std::string>>& getMemberVariables() const;
        bool hasFunction(const std::string& funcName);
        const std::vector<FunctionType*>* getFunction(const std::string& funcName);
        void addFunction(FunctionType* func);
        void addConstructor(FunctionType* func);
        void setDestructor(FunctionType* func);

    private:
        std::vector<std::pair<Type*, std::string>> memberList_;
        std::map<std::string, std::vector<FunctionType*>> memberFunctions_;
        std::vector<FunctionType*> constructors_;
        FunctionType* destructor_;
    };
}
