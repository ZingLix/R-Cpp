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
    private:
        std::unique_ptr<SymbolTable> symbol_table_;
        std::vector<std::unique_ptr<ClassAST>> classes_;
        int64_t nameless_var_count_;
    };
}