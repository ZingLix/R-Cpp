#pragma once
#include <string>
#include <vector>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

class CodeGenerator;

struct Variable
{
    Variable() :alloc(nullptr){}
    Variable(const std::string& n,const std::string& t,llvm::AllocaInst* a=nullptr,bool isCons=false)
        :name(n),type(t),alloc(a),isConst(isCons)
    { }

    std::string name;
    std::string type;
    llvm::AllocaInst* alloc;
    bool isConst;
};

struct Function
{
    Function() :alloc(nullptr) {}
    Function(const std::string& n, std::vector<Variable>& t,
        const std::string& retType="i32" , llvm::Function* a=nullptr)
        :name(n), args(t),returnType(retType), alloc(a) {
    }

    std::string name;
    std::vector<Variable> args;
    std::string returnType;
    llvm::Function* alloc;
};

llvm::Type* get_builtin_type(const std::string& s, CodeGenerator& cg);

llvm::Value* get_builtin_type_default_value(const std::string& s, CodeGenerator& cg);

bool is_builtin_type(llvm::Type::TypeID type);
bool is_builtin_type(const std::string& s);