#pragma once
#include <string>
#include <llvm/IR/Type.h>
#include "CodeGenerator.h"

//enum class Type
//{
//    i32,i64,u32,u64,Bool,Float,Double,none
//};
//
llvm::Type* get_builtin_type(const std::string& s, CodeGenerator& cg);

llvm::Value* get_builtin_type_default_value(const std::string& s, CodeGenerator& cg);

bool is_builtin_type(llvm::Type::TypeID type);
