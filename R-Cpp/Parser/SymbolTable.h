#pragma once
#include "Type.h"
#include "../Util/Token.h"
#include <vector>
#include <map>

class BlockExprAST;
class ExprAST;
namespace Parse {
    class ASTContext;
    class ClassDecl;

    class Variable
    {
    public:
        Variable(Type* type,const std::string& varName):type_(type),name_(varName)
        { }

        Type* type_;
        std::string name_;
    };

    struct ClassTemplate
    {
      //  ClassTemplate(){}
        ClassTemplate(std::vector<std::pair<std::string, std::string>> typelist, std::unique_ptr<ClassDecl> decl);
        ClassTemplate(const std::string& name, std::vector<std::pair<std::string, std::string>> typelist);

        std::string name;
        std::vector<std::pair<std::string, std::string>> typeList;
        std::unique_ptr<ClassDecl> classDecl_;
        std::vector<std::pair<std::vector<Type*>, std::unique_ptr<Type>>> instantiatedType;

        Type* instantiate(const std::vector<Type*>& args, ASTContext* context);
    };

    struct NamespaceHelper
    {
        NamespaceHelper(const std::string& nsName, NamespaceHelper* last)
            :name(nsName), lastNS(last) {
        }

        void createNewNS(const std::string& name) {
            nextNS[name] = std::make_unique<NamespaceHelper>(name, this);
        }

        std::string name;
        std::map<std::string, std::unique_ptr<NamespaceHelper>> nextNS;
        std::vector<NamespaceHelper*> insertedNS;
        std::map<std::string, std::vector<std::unique_ptr<FunctionType>>> namedFunction;
        std::map<std::string, std::unique_ptr<Type>> namedType;
        std::map<std::string, ClassTemplate> classTemplate;
        std::map<std::string, Type*> alias;
        NamespaceHelper* lastNS;
    };

    class SymbolTable
    {
    public:
        SymbolTable(ASTContext& context);
        void createScope();
        void destroyScope();
        void createNamespace(const std::string& name);
        void destroyNamespace();

        Variable* getVariable(const std::string& name);
        void addVariable(Type*type, const std::string& name);
        CompoundType* addType(const std::string& name, std::vector<std::pair<Type*, std::string>> memberList);
        Type* getType(const std::string& name,const std::vector<Type*>& args={});
        FunctionType* addFunction(const std::string& name, std::vector<std::pair<Type*, std::string>> argList,Type* returnType,Type* classType=nullptr,bool isExternal=false);
        const std::vector<std::unique_ptr<FunctionType>>* getFunction(const std::string& name, const std::vector<std::string>& ns_hierarchy={});
        const std::vector<std::string>& getNamespaceHierarchy();
        NamespaceHelper* getNamespace(const std::string& name) const;
        NamespaceHelper* currentNamespace() const
        {
            return cur_namespace_;
        }
        //void addClassTemplate(ClassTemplate template_);
        void addClassTemplate(std::vector<std::pair<std::string, std::string>> arglist, std::unique_ptr<ClassDecl> decl);
        //ClassTemplate getClassTemplate(std::string name);
        //std::string getMangledClassName(VarType type);
        void setAlias(const std::string& newName, Type* oldType);
        void unsetAlias(const std::string& name);
       // std::vector<std::unique_ptr<ExprAST>> callDestructor();
       // void callNamelessVarDestructor(std::vector<std::unique_ptr<ExprAST>>& exprs);
        void addNamelessVariable(Type* type,const std::string name)
        {
            nameless_values_.push_back(std::make_unique<Variable>(type,name));
        }
        void setSpecfiedNamespace(NamespaceHelper* ns);
        void unsetSpecfiedNamespace();

        class ScopeGuard
        {
        public:
            ScopeGuard(SymbolTable& st);
            ~ScopeGuard();
            void setBlock(BlockExprAST* block);

        private:
            SymbolTable& st_;
            BlockExprAST* block_;
        };

        class NamespaceGuard
        {
        public:
            NamespaceGuard(SymbolTable& st, const std::string& name);
            ~NamespaceGuard();

        private:
            SymbolTable& st_;
        };

    private:
        const std::vector<std::unique_ptr<FunctionType>>* getRawFunction_(const std::string& name, const std::vector<std::string>& ns_hierarchy, NamespaceHelper* ns);
        //ClassTemplate getClassTemplate_(std::string name, NamespaceHelper* ns);
        //void callDestructorForCurScope(BlockExprAST* block);

        ASTContext& context_;
        std::vector<std::map<std::string, std::unique_ptr<Variable>>> named_values_;
        std::vector<std::vector<std::string>> named_values_seq_;
        std::vector<std::unique_ptr<Variable>> nameless_values_;
        NamespaceHelper helper_;
        NamespaceHelper* cur_namespace_;
        NamespaceHelper* specfied_namespace_;
        std::vector<std::string> ns_hierarchy_;
    };
}