#pragma once
#include <map>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

namespace CG {
    class CodeGenerator;

    struct ClassSymbol
    {
        ClassSymbol():type(nullptr)
        { }

        ClassSymbol(llvm::Type* type)
            :type(type)
        {}

        llvm::Type* type;
        // <type, name>
        std::vector<std::pair<std::string, std::string>> members;
    };

    class SymbolTable
    {
    public:
        SymbolTable(CodeGenerator&cg):cg_(cg)
        {
            createScope();
            generateBuiltinType();
        }
        void createScope() {
            var_map_.emplace_back();
        }
        void destroyScope() {
            var_map_.pop_back();
        }

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

        llvm::AllocaInst* getAlloc(const std::string& name)
        {
            for(auto it=var_map_.rbegin();it!=var_map_.rend();++it)
            {
                auto iterator = it->find(name);
                if (iterator != it->end()) return iterator->second;
            }
            return nullptr;
        }

        void setAlloc(const std::string& name,llvm::AllocaInst* alloc)
        {
            var_map_.back()[name] = alloc;
        }

        llvm::Function* getFunction(const std::string& name)
        {
            return function_map_[name];
        }

        void setFunction(const std::string& name,llvm::Function* func)
        {
            function_map_[name] = func;
        }

        llvm::Type* getType(const std::string& name);

        void setType(const std::string& name,llvm::StructType* type)
        {
            class_map_[name].type = type;
        }

        const ClassSymbol& getClass(const std::string& name)
        {
            return class_map_[name];
        }

        void setClass(const std::string& name, ClassSymbol symbol)
        {
            class_map_[name] = symbol;
        }

    private:
        void generateBuiltinType();

        CodeGenerator& cg_;
        std::map<std::string, llvm::Function*> function_map_;
        std::map<std::string, ClassSymbol> class_map_;
        std::vector<std::map<std::string, llvm::AllocaInst*>> var_map_;
    };
}
