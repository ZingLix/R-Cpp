#include "SymbolTable.h"
#include "CodeGenerator.h"

llvm::Type* CG::SymbolTable::getType(const std::string& name)
{
    if (name.find("__ptr") == 0)
    {
        size_t pos = 6;
        assert(name[pos] == 'T');
        size_t count = 0;
        while (isdigit(name[++pos]))
        {
            count = count * 10 + name[pos] - '0';
        }
        std::string type(name.begin() + pos, name.end());
        return llvm::PointerType::getUnqual(getType(type));
    }
    if (name.find("__arr") == 0)
    {
        size_t pos = 6;
        assert(name[pos] == 'T');
        size_t count = 0;
        while (isdigit(name[++pos]))
        {
            count = count * 10 + name[pos] - '0';
        }
        std::string type(name.begin() + pos, name.begin() + pos + count);
        pos += count;
        assert(name[pos] == 'I');
        std::string amount(name.begin() + pos + 1, name.end());
        return llvm::ArrayType::get(getType(type), std::stoi(amount));
    }
    //if(is_builtin_type(name))
    //{
    //    return get_builtin_type(name, cg_);
    //}
    return class_map_[name].type;
}

void CG::SymbolTable::generateBuiltinType()
{
    class_map_["i32"] = ClassSymbol(llvm::Type::getInt32Ty(cg_.context()));
    class_map_["i64"] = ClassSymbol(llvm::Type::getInt64Ty(cg_.context()));
    class_map_["u32"] = ClassSymbol(llvm::Type::getInt32Ty(cg_.context()));
    class_map_["u64"] = ClassSymbol(llvm::Type::getInt64Ty(cg_.context()));
    class_map_["bool"] = ClassSymbol(llvm::Type::getInt1Ty(cg_.context()));
    class_map_["float"] = ClassSymbol(llvm::Type::getFloatTy(cg_.context()));
    class_map_["double"] = ClassSymbol(llvm::Type::getDoubleTy(cg_.context()));
    class_map_["void"] = ClassSymbol(llvm::Type::getVoidTy(cg_.context()));
}
