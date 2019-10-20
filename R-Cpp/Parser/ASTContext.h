#pragma once
#include "SymbolTable.h"
namespace Parse
{
    class ASTContext
    {
    public:
        ASTContext():symbol_table_(std::make_unique<SymbolTable>()),nameless_var_count_(1) {
            
        }
        SymbolTable& symbolTable() {
            return *symbol_table_;
        }
        int64_t getNamelessVarCount() {
            return nameless_var_count_++;
        }
        void addClassAST(std::unique_ptr<ClassAST> ast)
        {
            assert(ast != nullptr);
            classes_.push_back(std::move(ast));
        }
        void addType(const std::string& name, std::vector<std::pair<Type*, std::string>> memberList)
        {
            auto t = symbol_table_->addType(name, std::move(memberList));

            std::vector<std::pair<std::string, std::string>> members;
            for(auto& m:t->getMemberVariables())
            {
                members.emplace_back(m.first->mangledName(), m.second);
            }
            classes_.push_back(std::make_unique<ClassAST>(t->mangledName(), std::move(members), t));
        }
        FunctionType* addFuncPrototype(const std::string& name, std::vector<std::pair<Type*, std::string>> argList, Type* returnType, bool isExternal = false)
        {
            std::vector<std::pair<std::string, std::string>> members;
            for (auto& m : argList) {
                members.emplace_back(m.first->mangledName(), m.second);
            }
            auto fn = symbol_table_->addFunction(name, std::move(argList), returnType, isExternal);
            prototype_.push_back(std::make_unique<PrototypeAST>(fn->mangledName(), std::move(members), returnType->mangledName()));
            return fn;
        }
        void setFuncBody(FunctionType* func,std::unique_ptr<BlockExprAST> body)
        {
            functions_.push_back(std::make_unique<FunctionAST>(func->mangledName(), std::move(body)));
        }
        std::vector<std::unique_ptr<ClassAST>>* Class()
        {
            return &classes_;
        }
        std::vector<std::unique_ptr<PrototypeAST>>* Prototype()
        {
            return &prototype_;
        }
        std::vector<std::unique_ptr<FunctionAST>>* Function()
        {
            return &functions_;
        }
    private:
        std::unique_ptr<SymbolTable> symbol_table_;
        std::vector<std::unique_ptr<ClassAST>> classes_;
        std::vector<std::unique_ptr<PrototypeAST>> prototype_;
        std::vector<std::unique_ptr<FunctionAST>> functions_;
        int64_t nameless_var_count_;
    };
}