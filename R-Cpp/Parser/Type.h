#pragma once
#include <string>
#include <set>
#include <vector>
#include <memory>
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
        const std::string getTypename()
        {
            return name_;
        }
        virtual std::string mangledName() = 0;
        virtual ~Type(){}
    
        Type(const std::string& name, std::vector<Type*> typelist={})
            :name_(name),typelist_(std::move(typelist)),namespaceHierarchy_(nullptr)
        { }
        void setNamespaceHierarchy(NamespaceHelper* hierarchy)
        {
            namespaceHierarchy_ = hierarchy;
        }
        NamespaceHelper* getNamespaceHierarchy() const
        {
            return namespaceHierarchy_;
        }

    protected:
        std::string name_;
        std::vector<Type*> typelist_;
        NamespaceHelper* namespaceHierarchy_;
    };

    class BuiltinType : public Type
    {
    public:
        BuiltinType(const std::string& typeName, std::vector<Type*> typelist = {})
            :Type(typeName)
        {
            if (builtinTypeSet_.find(typeName) == builtinTypeSet_.end())
                throw std::logic_error(typeName+ " is not builtin type.");
        }

        std::string mangledName() override
        {
            return getTypename();
        }
    private:
        static const std::set<std::string> builtinTypeSet_;
    };

    class FunctionType : public Type
    {
    public:
        FunctionType(const std::string& functionName, std::vector<std::pair<Type*, std::string>> argTypeList,
            Type* returnType,Type* classType,bool isExternal=false, std::vector<Type*> typelist = {})
            :Type(functionName, typelist),argTypeList_(std::move(argTypeList)),returnType_(returnType),classType_(classType),isExternal_(isExternal)
        {
            
        }
        std::string mangledName() override
        {
            auto name = getTypename();
            if (name == "main") return "main";
            if (name == "_start") return "_start";
            if (isExternal_) return name;
            std::string mangledName = "_";
            mangledName += isExternal_ ? "Z" : "R";
            if (classType_ != nullptr) mangledName += classType_->mangledName();
            mangledName += std::to_string(name.length()) + name;
            for (auto& v : argTypeList_) {
                mangledName += v.first->mangledName();
            }
            return mangledName;
        }
        const std::vector<std::pair<Type*, std::string>>& args()
        {
            return argTypeList_;
        }
        Type* returnType()
        {
            return returnType_;
        }

    private:
        std::vector<std::pair<Type*,std::string>> argTypeList_;
        Type* returnType_;
        Type* classType_;
        bool isExternal_;
    };

    class CompoundType: public Type
    {
    public:
        CompoundType(const std::string& typeName, std::vector<std::pair<Type*, std::string>> memberList, std::vector<Type*> typelist = {})
            :Type(typeName,typelist),memberList_(std::move(memberList))
        { }
        std::string mangledName() override
        {
            auto res = getTypename();
            //if (type.templateArgs.size() != 0) {
            //    res += "_";
            //    for (auto& v : type.templateArgs) {
            //        auto m = mangle(v);
            //        if (isdigit(m[0])) {
            //            res += "I" + m;
            //        } else {
            //            res += "T" + std::to_string(m.length()) + mangle(v);
            //        }
            //    }
            //}
            return res;
        }
        Type* getMemberType(const std::string& name)
        {
            for(auto& pair:memberList_)
            {
                if (pair.second == name) return pair.first;
            }
            return nullptr;
        }

        int getMemberIndex(const std::string& name)
        {
            for(auto it=memberList_.begin();it!=memberList_.end();++it)
            {
                if (it->second == name) return it - memberList_.begin();
            }
            return -1;
        }

        const std::vector<FunctionType*>& getConstructors()
        {
            return constructors_;
        }

        FunctionType* getDestructor()
        {
            return destructor_;
        }

        const std::vector<std::pair<Type*, std::string>>& getMemberVariables()
        {
            return memberList_;
        }

    private:
        std::vector<std::pair<Type*, std::string>> memberList_;
        std::vector<FunctionType*> memberFunctions_;
        std::vector<FunctionType*> constructors_;
        FunctionType* destructor_;
    };
}
