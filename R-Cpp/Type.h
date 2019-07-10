#pragma once
#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

class CodeGenerator;

struct Variable
{
    Variable() :alloc(nullptr){}
    Variable(const std::string& n,const std::string& t,llvm::AllocaInst* a,bool isCons=false)
        :name(n),type(t),alloc(a),isConst(isCons)
    { }

    std::string name;
    std::string type;
    llvm::AllocaInst* alloc;
    bool isConst;
};

llvm::Type* get_builtin_type(const std::string& s, CodeGenerator& cg);

llvm::Value* get_builtin_type_default_value(const std::string& s, CodeGenerator& cg);

bool is_builtin_type(llvm::Type::TypeID type);
