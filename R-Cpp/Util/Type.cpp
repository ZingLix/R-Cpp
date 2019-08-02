#include "Type.h"
#include "../CodeGenerator/CodeGenerator.h"

bool VarType::operator==(const VarType& other) const
{
    if (typeName == other.typeName)
    {
        if (templateArgs.size() == other.templateArgs.size())
        {
            size_t i = 0;
            while (i < templateArgs.size())
            {
                if (templateArgs[i] != other.templateArgs[i])
                    return false;
            }
            return true;
        }
        return false;
    }
    return false;
}

bool VarType::operator!=(const VarType& other) const
{
    return !operator==(other);
}

bool VarType::operator<(const VarType& other) const
{
    if (typeName == other.typeName) {
        if (templateArgs.size() == other.templateArgs.size()) {
            size_t i = 0;
            while (i < templateArgs.size()) {
                if (templateArgs[i] != other.templateArgs[i])
                    return templateArgs[i]<other.templateArgs[i];
                ++i;
            }
            return false;
        }
        return templateArgs.size()<other.templateArgs.size();
    }
    return typeName<other.typeName;
}

std::string VarType::mangle(const VarType& type)
{
    auto res = type.typeName;
    if(type.templateArgs.size()!=0)
    {
        res += "_";
        for(auto& v:type.templateArgs)
        {
            auto m = mangle(v);
            if(isdigit(m[0]))
            {
                res += "I" + m;
            }else
            {
                res +="T"+ std::to_string(m.length()) + mangle(v);
            }
        }
    }
    return res;
}

llvm::Type* get_builtin_type(const std::string& s, CG::CodeGenerator& cg) {
    if (s == "i32") return llvm::Type::getInt32Ty(cg.context());
    if (s == "i64") return llvm::Type::getInt64Ty(cg.context());
    if (s == "u32") return llvm::Type::getInt32Ty(cg.context());
    if (s == "u64") return llvm::Type::getInt64Ty(cg.context());
    if (s == "bool") return llvm::Type::getInt1Ty(cg.context());
    if (s == "float") return llvm::Type::getFloatTy(cg.context());
    if (s == "double") return llvm::Type::getDoubleTy(cg.context());
    if (s == "void") return llvm::Type::getVoidTy(cg.context());
    return nullptr;
}

llvm::Value* get_builtin_type_default_value(const std::string& s, CG::CodeGenerator& cg) {
    if (s == "i32") return llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, true));
    if (s == "i64") return llvm::ConstantInt::get(cg.context(), llvm::APInt(64, 0, true));
    if (s == "u32") return llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, false));
    if (s == "u64") return llvm::ConstantInt::get(cg.context(), llvm::APInt(32, 0, false));
    if (s == "bool") return llvm::ConstantInt::get(cg.context(), llvm::APInt(1, 0, false));
    if (s == "float") return llvm::ConstantFP::get(cg.context(), llvm::APFloat(0.0));
    if (s == "double") return llvm::ConstantFP::get(cg.context(), llvm::APFloat(0.0));
    return nullptr;
}

bool is_builtin_type(llvm::Type::TypeID type) {
    switch (type) {
    case llvm::Type::TypeID::FloatTyID:
    case llvm::Type::TypeID::DoubleTyID:
    case llvm::Type::TypeID::IntegerTyID:
    case llvm::Type::TypeID::VoidTyID:
        return true;
    default:
        return false;
    }
}

bool is_builtin_type(const std::string& s) {
    return (s == "i32" || s == "i64" || s == "u32" ||
        s == "u64" || s == "bool" || s == "float" ||
        s == "double" || s == "__arr"||s=="void"||s=="__ptr");
}

std::string Function::mangle(const Function& F)
{
    if (F.name == "main") return "main";
    if (F.name == "_start") return "_start";
    if (F.isExternal) return F.name;
    std::string mangledName = "_";
    mangledName += F.isExternal ? "Z" : "R";
    if (F.classType.typeName != "") mangledName += mangle(F.classType);
    mangledName += mangle(F.name);
    for(auto& v:F.args)
    {
        mangledName += mangle(v.type);
    }
    return mangledName;
}

std::string Function::mangle(const std::string& name)
{
    return std::to_string(name.length()) + name;
}

std::string Function::mangle(const VarType& type)
{
    return "I" + std::to_string(type.typeName.length()) + type.typeName;
}
