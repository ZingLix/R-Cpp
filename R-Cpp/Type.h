#pragma once
#include <string>
#include <vector>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

class CodeGenerator;

struct VarType
{
    VarType():isConst(false){}
    VarType(std::string type,bool isconst=false)
        :typeName(type),isConst(isconst)
    {
        
    }

    std::string typeName;
    std::vector<VarType> templateArgs;
    bool isConst;
};

struct Variable
{
    Variable() :alloc(nullptr){}
    Variable(const std::string& n,const VarType& t,llvm::AllocaInst* a=nullptr)
        :name(n),type(t),alloc(a)
    { }

    std::string name;
    VarType type;
    llvm::AllocaInst* alloc;
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

struct Class
{
    Class(){}

    Class(const std::string& n)
        :name(n){
    }

    std::string name;
    std::vector<Variable> memberVariables;
    std::vector<::Function> memberFunctions;
    llvm::Type* type;
};



llvm::Type* get_builtin_type(const std::string& s, CodeGenerator& cg);
llvm::Type* get_type(VarType t, CodeGenerator& cg);

llvm::Value* get_builtin_type_default_value(const std::string& s, CodeGenerator& cg);

bool is_builtin_type(llvm::Type::TypeID type);
bool is_builtin_type(const std::string& s);