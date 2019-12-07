#pragma once
#include "SymbolTable.h"
#include "../CodeGenerator/AST.h"

namespace Parse
{
    class ASTContext
    {
    public:
        ASTContext();

        SymbolTable& symbolTable();

        int64_t getNamelessVarCount();
        void addClassAST(std::unique_ptr<ClassAST> ast);
        Type* addType(const std::string& name, std::vector<std::pair<Type*, std::string>> memberList);
        FunctionType* addFuncPrototype(const std::string& name, std::vector<std::pair<Type*, std::string>> argList,
                                       Type* returnType, bool isExternal = false);
        void setFuncBody(FunctionType* func, std::unique_ptr<BlockExprAST> body);
        LiteralType* addLiteralType(LiteralType::category type, int64_t val);
        void addClassTemplate(std::vector<std::pair<std::string, std::string>> arglist, std::unique_ptr<ClassDecl> decl);
        std::vector<std::unique_ptr<ClassAST>>* Class();
        std::vector<std::unique_ptr<PrototypeAST>>* Prototype();
        std::vector<std::unique_ptr<FunctionAST>>* Function();
        CompoundType* currentClass() const;
        void setCurrentClass(CompoundType* t);
        void unsetCurrentClass();
        std::string namelessVarName();
        void addLLVMType(CompoundType* t);

        struct ClassScopeGuard
        {
        public:
            ClassScopeGuard(ASTContext& context, CompoundType* t):context_(context)
            {
                context.setCurrentClass(t);
            }
            ~ClassScopeGuard() {
                context_.unsetCurrentClass();
            }
        private:
            ASTContext& context_;
        };

    private:
        std::unique_ptr<SymbolTable> symbol_table_;
        std::vector<std::unique_ptr<ClassAST>> classes_;
        std::vector<std::unique_ptr<PrototypeAST>> prototype_;
        std::vector<std::unique_ptr<FunctionAST>> functions_;
        std::vector<std::unique_ptr<LiteralType>> literal_types_;
        int64_t nameless_var_count_;

        CompoundType* cur_parsing_class_;
    };
}
