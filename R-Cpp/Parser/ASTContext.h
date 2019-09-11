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
    private:
        std::unique_ptr<SymbolTable> symbol_table_;
        int64_t nameless_var_count_;
    };
}