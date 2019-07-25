#pragma once
#include <string>
#include <vector>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

class CodeGenerator;
class PrototypeAST;

struct VarType
{
    VarType():isConst(false){}
    VarType(std::string type,bool isconst=false)
        :typeName(type),isConst(isconst)
    { }

    bool operator==(const VarType& other) const;
    bool operator!=(const VarType& other) const;
    bool operator<(const VarType& other) const;

    std::string typeName;
    std::vector<VarType> templateArgs;
    std::vector<std::string> namespaceHierarchy;
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
    Function() :alloc(nullptr),isExternal(false) {}
    Function(const std::string& n, std::vector<Variable>& t,
        const VarType& retType, llvm::Function* a=nullptr)
        :name(n), args(t),returnType(retType), alloc(a),isExternal(false) {
    }

    static std::string mangle(const Function& F);
private:
    static std::string mangle(const std::string& name);
    static std::string mangle(const VarType& type);

public:
    std::string name;
    std::vector<Variable> args;
    VarType returnType;
    VarType classType;
    llvm::Function* alloc;
    bool isExternal;
};

struct Class
{
    Class(){}

    Class(const VarType& n)
        :type(n){
    }

    VarType type;
    std::vector<Variable> memberVariables;
    std::vector<::Function> memberFunctions;
    llvm::Type* type_llvm;
};

llvm::Type* get_builtin_type(const std::string& s, CodeGenerator& cg);
llvm::Type* get_type(VarType t, CodeGenerator& cg);

llvm::Value* get_builtin_type_default_value(const std::string& s, CodeGenerator& cg);

bool is_builtin_type(llvm::Type::TypeID type);
bool is_builtin_type(const std::string& s);