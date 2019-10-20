//#pragma once
//#include <string>
//#include <vector>
//#include <llvm/IR/Type.h>
//#include <llvm/IR/Instructions.h>
//
//namespace CG {
//    class CodeGenerator;
//}
//
//class PrototypeAST;
//
//struct VarType
//{
//    VarType():isConst(false){}
//    VarType(std::string type,bool isconst=false)
//        :typeName(type),isConst(isconst)
//    { }
//
//    bool operator==(const VarType& other) const;
//    bool operator!=(const VarType& other) const;
//    bool operator<(const VarType& other) const;
//
//    static std::string mangle(const VarType& type);
//    std::string mangledName() {
//        return mangle(*this);
//    }
//    std::string typeName;
//    std::vector<VarType> templateArgs;
//    std::vector<std::string> namespaceHierarchy;
//    bool isConst;
//};
//
//struct Variable
//{
//    Variable() {}
//    Variable(const std::string& n,const VarType& t)
//        :name(n),type(t)
//    { }
//
//    std::string name;
//    VarType type;
//};
//
//struct Function
//{
//    Function() :isExternal(false) {}
//    Function(const std::string& n, std::vector<Variable>& t,
//        const VarType& retType,bool external, llvm::Function* a=nullptr)
//        :name(n), args(t),returnType(retType),isExternal(external) {
//    }
//
//    static std::string mangle(const Function& F);
//    std::string mangledName()
//    {
//        return mangle(*this);
//    }
//private:
//    static std::string mangle(const VarType& type);
//    static std::string mangle(const std::string& name);
//
//public:
//    std::string name;
//    std::vector<Variable> args;
//    VarType returnType;
//    VarType classType;
//    bool isExternal;
//};
//
//struct Class
//{
//    Class(){}
//
//    Class(const VarType& n)
//        :type(n){
//    }
//
//    VarType type;
//    std::vector<Variable> memberVariables;
//    std::vector<::Function> memberFunctions;
//    std::vector<::Function> constructors;
//    ::Function destructor;
//};
//
//llvm::Type* get_builtin_type(const std::string& s, CG::CodeGenerator& cg);
////llvm::Type* get_type(VarType t, CG::CodeGenerator& cg);
//
//llvm::Value* get_builtin_type_default_value(const std::string& s, CG::CodeGenerator& cg);
//
//bool is_builtin_type(llvm::Type::TypeID type);
//bool is_builtin_type(const std::string& s);