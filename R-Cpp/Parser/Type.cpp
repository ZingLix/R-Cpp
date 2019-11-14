#include "Type.h"
#include "../CodeGenerator/CodeGenerator.h"

const std::set<std::string> Parse::BuiltinType::builtinTypeSet_
    = { "i32","i64","u32","u64","bool","float","double","void" };

const std::string Parse::Type::getTypename() const {
    return name_;
}

Parse::Type::Type(const std::string& name, std::vector<Type*> typelist): name_(name), typelist_(std::move(typelist)),
                                                                         namespaceHierarchy_(nullptr) {
}

void Parse::Type::setNamespaceHierarchy(NamespaceHelper* hierarchy) {
    namespaceHierarchy_ = hierarchy;
}

Parse::NamespaceHelper* Parse::Type::getNamespaceHierarchy() const {
    return namespaceHierarchy_;
}

Parse::BuiltinType::BuiltinType(const std::string& typeName, std::vector<Type*> typelist): Type(
    typeName, std::move(typelist)) {
    //if (builtinTypeSet_.find(typeName) == builtinTypeSet_.end())
    //    throw std::logic_error(typeName+ " is not builtin type.");
}

std::string Parse::BuiltinType::mangledName() {
    auto res = getTypename();
    if (typelist_.size() != 0) {
        res += "_";
        for (auto& v : typelist_) {
            auto m = v->mangledName();
            if (isdigit(m[0])) {
                res += "I" + m;
            } else {
                res += "T" + std::to_string(m.length()) + m;
            }
        }
    }
    return res;
}

const std::set<std::string>& Parse::BuiltinType::builtinTypeSet() {
    return builtinTypeSet_;
}

Parse::FunctionType::FunctionType(const std::string& functionName,
                                  std::vector<std::pair<Type*, std::string>> argTypeList, Type* returnType,
                                  Type* classType, bool isExternal,
                                  std::vector<Type*> typelist): Type(functionName, typelist),
                                                                argTypeList_(std::move(argTypeList)),
                                                                returnType_(returnType), classType_(classType),
                                                                isExternal_(isExternal) {

}

std::string Parse::FunctionType::mangledName() {
    auto name = getTypename();
    if (name == "main") return "main";
    if (name == "_start") return "_start";
    if (isExternal_) return name;
    std::string mangledName = "_";
    mangledName += isExternal_ ? "Z" : "R";
    if (classType_ != nullptr) mangledName += classType_->mangledName();
    mangledName += std::to_string(name.length()) + name;
    for (auto& v : argTypeList_) {
        mangledName += v.first->mangledName();
    }
    return mangledName;
}

const std::vector<std::pair<Parse::Type*, std::string>>& Parse::FunctionType::args() {
    return argTypeList_;
}

Parse::Type* Parse::FunctionType::returnType() {
    return returnType_;
}

Parse::Type* Parse::FunctionType::classType() const {
    return classType_;
}

Parse::CompoundType::CompoundType(const std::string& typeName, std::vector<std::pair<Type*, std::string>> memberList,
                                  std::vector<Type*> typelist): Type(typeName, typelist),
                                                                memberList_(std::move(memberList)) {
}

std::string Parse::CompoundType::mangledName() {
    auto res = getTypename();
    if (typelist_.size() != 0) {
        res += "_";
        for (auto& v : typelist_) {
            auto m = v->mangledName();
            if (isdigit(m[0])) {
                res += "I" + m;
            } else {
                res += "T" + std::to_string(m.length()) + m;
            }
        }
    }
    return res;
}

Parse::Type* Parse::CompoundType::getMemberType(const std::string& name) {
    for (auto& pair : memberList_) {
        if (pair.second == name) return pair.first;
    }
    return nullptr;
}

int Parse::CompoundType::getMemberIndex(const std::string& name) {
    for (auto it = memberList_.begin(); it != memberList_.end(); ++it) {
        if (it->second == name) return it - memberList_.begin();
    }
    return -1;
}

const std::vector<Parse::FunctionType*>* Parse::CompoundType::getConstructors() const {
    return &constructors_;
}

Parse::FunctionType* Parse::CompoundType::getDestructor() const {
    return destructor_;
}

const std::vector<std::pair<Parse::Type*, std::string>>& Parse::CompoundType::getMemberVariables() const {
    return memberList_;
}

bool Parse::CompoundType::hasFunction(const std::string& funcName) {
    return memberFunctions_.find(funcName) != memberFunctions_.end();
}

const std::vector<Parse::FunctionType*>* Parse::CompoundType::getFunction(const std::string& funcName) {
    auto it = memberFunctions_.find(funcName);
    if (it == memberFunctions_.end()) return nullptr;
    return &(it->second);
}

void Parse::CompoundType::addFunction(FunctionType* func) {
    memberFunctions_[func->getTypename()].push_back(func);
}

void Parse::CompoundType::addConstructor(FunctionType* func) {
    constructors_.push_back(func);
}

void Parse::CompoundType::setDestructor(FunctionType* func) {
    destructor_ = func;
}
