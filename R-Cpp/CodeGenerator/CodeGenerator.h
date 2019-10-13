#pragma once
#include <memory>
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"

#include "../Parser/Parser.h"
#include "SymbolTable.h"

namespace CG {
    class SymbolTable;

    class CodeGenerator
    {
    public:
        CodeGenerator(Parse::Parser&);

        llvm::LLVMContext& context();
        llvm::IRBuilder<>& builder();
        llvm::Function* getFunction(const std::string& Callee);
        llvm::Module& getModule();
        llvm::legacy::FunctionPassManager* FPM();

        void generate();
        void output();

        SymbolTable& symbol() { return st_; }

        llvm::Type* getBuiltinType(const std::string& name);
        llvm::Value* getBuiltinTypeDefaultValue(const std::string& name);
        llvm::Type* getType(const std::string& name)
        {
            return symbol().getType(name);
        }
        llvm::Type* getPtrTypeOf(const std::string& name)
        {
            return llvm::PointerType::getUnqual(getType(name));
        }
        llvm::Type* getArrayTypeOf(const std::string& name,int count)
        {
            return llvm::ArrayType::get(getType(name), count);
        }
    private:
        Parse::Parser& parser_;
        llvm::LLVMContext TheContext;
        llvm::IRBuilder<> Builder;
        std::unique_ptr<llvm::Module> TheModule;
        std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
        SymbolTable st_;
    };
}
