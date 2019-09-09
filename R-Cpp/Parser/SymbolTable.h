#pragma once
#include "../Util/Type.h"
#include "../Util/Token.h"

class BlockExprAST;
class ExprAST;
namespace Parse {

    struct ClassTemplate
    {
        VarType type;
        std::vector<std::pair<VarType, std::string>> typeList;
        std::vector<Token> token;
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
        std::map<std::string, std::vector<Function>> namedFunction;
        std::map<std::string, Class> namedClass;
        std::map<std::string, ClassTemplate> classTemplate;
        std::map<VarType, VarType> alias;
        NamespaceHelper* lastNS;
    };

    class Parser;

    class SymbolTable
    {
    public:
        SymbolTable(Parser& p);
        void createScope();
        void destroyScope();
        void createNamespace(const std::string& name);
        void destroyNamespace();

        Variable getValue(const std::string& name);
        void setValue(const std::string& name, Variable val);
        bool hasType(const std::string& name);
        void addClass(const std::string& name, Class c);
        Class getClass(const std::string& name);
        VarType getClassMemberType(const std::string& classType, const std::string& memberName);
        int getClassMemberIndex(const std::string& classType, const std::string& memberName);
        void addFunction(const ::Function& func);
        const std::vector<::Function>* getFunction(const std::string& name, const std::vector<std::string>& ns_hierarchy = {});
        const std::vector<std::string>& getNamespaceHierarchy();
        void addClassTemplate(ClassTemplate template_);
        ClassTemplate getClassTemplate(std::string name);
        std::string getMangledClassName(VarType type);
        VarType getVarType(VarType type);
        void setAlias(VarType newType, VarType oldType);
       // std::vector<std::unique_ptr<ExprAST>> callDestructor();
       // void callNamelessVarDestructor(std::vector<std::unique_ptr<ExprAST>>& exprs);
        void addNamelessVariable(Variable v)
        {
            nameless_values_.push_back(v);
        }

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
        const std::vector<Function>* getRawFunction_(const std::string& name, const std::vector<std::string>& ns_hierarchy, NamespaceHelper* ns);
        ClassTemplate getClassTemplate_(std::string name, NamespaceHelper* ns);
       // void callDestructorForCurScope(BlockExprAST* block);

        Parser& parser_;
        std::vector<std::map<std::string, Variable>> named_values_;
        std::vector<std::vector<std::string>> named_values_seq_;
        std::vector<Variable> nameless_values_;
        NamespaceHelper helper_;
        NamespaceHelper* cur_namespace_;
        std::vector<std::string> ns_hierarchy_;
    };
}