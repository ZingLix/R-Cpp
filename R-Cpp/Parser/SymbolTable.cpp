#include "SymbolTable.h"
#include "../CodeGenerator/CodeGenerator.h"

using namespace Parse;

ClassTemplate::
ClassTemplate(std::vector<std::pair<std::string, std::string>> typelist, std::unique_ptr<ClassDecl> decl):
    name(decl->name()), typeList(std::move(typelist)), classDecl_(std::move(decl))
{
    assert(classDecl_ != nullptr);
}

ClassTemplate::
ClassTemplate(const std::string& name, std::vector<std::pair<std::string, std::string>> typelist): name(name),
                                                                                                   typeList(std::move(
                                                                                                       typelist)),
                                                                                                   classDecl_(nullptr)
{
    // only for builtin type
    assert(name == "__ptr" || name == "__arr");
}

Type* ClassTemplate::instantiate(const std::vector<Type*>& args,ASTContext* context)
{
    if (args.size() != typeList.size())
    {
        throw std::logic_error("Template arguments count mismatch.");
    }
    for(auto& p:instantiatedType)
    {
        if(p.first==args)
        {
            return p.second.get();
        }
    }
    if(!classDecl_)
    {
        auto type = std::make_unique<BuiltinType>(name, args);
        auto ret = type.get();
        instantiatedType.emplace_back(args, std::move(type));
        return ret;
    }
    else
    {
        for(size_t i=0;i<typeList.size();++i)
        {
            context->symbolTable().setAlias(typeList[i].second, args[i]);
        }
        auto type = std::make_unique<CompoundType>(name, classDecl_->memberTypeList(context),args);
        type->setNamespaceHierarchy(context->symbolTable().currentNamespace());
        auto t = type.get();
        classDecl_->setType(t);
        instantiatedType.emplace_back(args, std::move(type));
        context->addLLVMType(t);
        classDecl_->registerMemberFunction(context);
        for(auto&p:typeList)
        {
            context->symbolTable().unsetAlias(p.second);
        }
        return t;
    }

}

SymbolTable::SymbolTable(ASTContext& context):context_(context), helper_("",nullptr),cur_namespace_(&helper_),specfied_namespace_(nullptr)
{
    createScope();
    for(auto& s:BuiltinType::builtinTypeSet())
    {
        helper_.namedType.emplace(s,std::make_unique<BuiltinType>(s));
    }
    std::vector<std::pair<std::string, std::string>> typelist;
    typelist.emplace_back("Any", "T");
    helper_.classTemplate.emplace("__ptr",ClassTemplate("__ptr",typelist));
    typelist.emplace_back("Integer", "Size");
    helper_.classTemplate.emplace("__arr", ClassTemplate("__arr", typelist));
}

void SymbolTable::createScope()
{
    named_values_.emplace_back();
    named_values_seq_.emplace_back();
}

void SymbolTable::destroyScope()
{
    named_values_.pop_back();
    named_values_seq_.pop_back();
}

void SymbolTable::createNamespace(const std::string& name)
{
    cur_namespace_->createNewNS(name);
    ns_hierarchy_.push_back(name);
    cur_namespace_ = cur_namespace_->nextNS[name].get();
}

void SymbolTable::destroyNamespace()
{
    ns_hierarchy_.pop_back();
    cur_namespace_ = cur_namespace_->lastNS;
}


Variable* SymbolTable::getVariable(const std::string& name)
{
    for (auto it = named_values_.rbegin(); it != named_values_.rend(); ++it)
    {
        auto t = it->find(name);
        if (t != it->end()) return t->second.get();
    }
    return nullptr;
}

void SymbolTable::addVariable(Type* type, const std::string& name)
{
    named_values_.back()[name] = std::make_unique<Variable>(type,name);
    named_values_seq_.back().push_back(name);
}

CompoundType* SymbolTable::addType(const std::string& name, std::vector<std::pair<Type*, std::string>> memberList)
{
    auto type = std::make_unique<CompoundType>(name, std::move(memberList));
    type->setNamespaceHierarchy(cur_namespace_);
    cur_namespace_->namedType[name] = std::move(type);
    return dynamic_cast<CompoundType*>(cur_namespace_->namedType[name].get());
}

Type* SymbolTable::getType(const std::string& t, const std::vector<Type*>& args)
{
    if(specfied_namespace_) {
        auto type = specfied_namespace_->namedType.find(t);
        if (type != specfied_namespace_->namedType.end()) return type->second.get();
        auto func = specfied_namespace_->namedFunction.find(t);
        if (func != specfied_namespace_->namedFunction.end()) return func->second[0].get();
        auto template_ = specfied_namespace_->classTemplate.find(t);
        if (template_ != specfied_namespace_->classTemplate.end()) {   //find by name
            if (template_->second.typeList.size() == args.size()) {   // check if the arg size is same
                for (auto& p : template_->second.instantiatedType) {   // check if has been instantiated
                    if (p.first == args) {
                        return p.second.get();
                    }
                }
                //instantiate template
                return template_->second.instantiate(args,&context_);
            }
        }
    }
    NamespaceHelper* ns = cur_namespace_;
    if(args.size()==0)
    {
        while (ns != nullptr) {
            auto alias = ns->alias.find(t);
            if (alias != ns->alias.end()) return alias->second;

            auto it = ns->namedType.find(t);
            if (it != ns->namedType.end()) return it->second.get();
            ns = ns->lastNS;
        }
        ns = cur_namespace_;
        while (ns != nullptr) {
            auto it = ns->namedFunction.find(t);
            if (it != ns->namedFunction.end()) {   //find by name
                return it->second[0].get();
            }
            ns = ns->lastNS;
        }
    }
    else
    {
        while (ns != nullptr) {
            auto it = ns->classTemplate.find(t);
            if (it != ns->classTemplate.end()) 
            {   //find by name
                if(it->second.typeList.size()==args.size())
                {   // check if the arg size is same
                    for(auto&p:it->second.instantiatedType)
                    {   // check if has been instantiated
                        if(p.first==args)
                        {
                            return p.second.get();
                        }
                    }
                    //instantiate template
                    return it->second.instantiate(args,&context_);
                }
            }
            ns = ns->lastNS;
        }
    }
    return nullptr;
}

FunctionType* SymbolTable::addFunction(const std::string& name, std::vector<std::pair<Type*, std::string>> argList, Type* returnType,Type* classType, bool isExternal)
{
    auto fn = std::make_unique<FunctionType>(name, std::move(argList), returnType, context_.currentClass(), isExternal);
    fn->setNamespaceHierarchy(cur_namespace_);
    auto ret = fn.get();
    cur_namespace_->namedFunction[name].push_back(std::move(fn));
    return ret;
}

const std::vector<std::unique_ptr<FunctionType>>* SymbolTable::getFunction(const std::string& name, const std::vector<std::string>& ns_hierarchy)
{
    if(specfied_namespace_) {
        return getRawFunction_(name, ns_hierarchy, specfied_namespace_);
    }
    auto cur = cur_namespace_;
    while (cur!=nullptr)
    {
        auto flist = getRawFunction_(name, ns_hierarchy, cur);
        if (flist->size() != 0) return flist;
        cur = cur->lastNS;
    }
    return nullptr;
}

const std::vector<std::string>& SymbolTable::getNamespaceHierarchy()
{
    return ns_hierarchy_;
}

NamespaceHelper* SymbolTable::getNamespace(const std::string& name) const {
    if(specfied_namespace_) {
        auto it = specfied_namespace_->nextNS.find(name);
        if (it != specfied_namespace_->nextNS.end()) return it->second.get();
        return nullptr;
    }
    auto cur = cur_namespace_;
    while (cur != nullptr) {
        auto it = cur->nextNS.find(name);
        if (it!=cur->nextNS.end()) return it->second.get();
        cur = cur->lastNS;
    }
    return nullptr;
}

const std::vector<std::unique_ptr<FunctionType>>* SymbolTable::getRawFunction_(const std::string& name, const std::vector<std::string>& ns_hier,NamespaceHelper* ns)
{
    for(size_t i=0;i<ns_hier.size();++i)
    {
        ns = ns->nextNS[ns_hier[i]].get();
        if (ns == nullptr) return {};
    }
    return &(ns->namedFunction[name]);
}

void SymbolTable::addClassTemplate(
    std::vector<std::pair<std::string, std::string>> arglist, std::unique_ptr<ClassDecl> decl)
{
    auto name = decl->name();
    cur_namespace_->classTemplate.emplace(std::make_pair(name, ClassTemplate(std::move(arglist), std::move(decl))));
//    cur_namespace_->classTemplate[decl->name()] = ClassTemplate(std::move(arglist),std::move(decl));
}

//ClassTemplate SymbolTable::getClassTemplate(std::string name)
//{
//    auto cur = cur_namespace_;
//    while (cur != nullptr) {
//        auto t = getClassTemplate_(name, cur);
//        if (t.token.size() != 0) return t;
//        cur = cur->lastNS;
//    }
//    return {};
//}
//
//ClassTemplate SymbolTable::getClassTemplate_(std::string name, NamespaceHelper* ns)
//{
//    return ns->classTemplate[name];
//}


void SymbolTable::setAlias(const std::string& newName, Type* oldType)
{
    cur_namespace_->alias[newName] = oldType;
}

void SymbolTable::unsetAlias(const std::string& name)
{
    cur_namespace_->alias.erase(cur_namespace_->alias.find(name));
}

//
//std::string SymbolTable::getMangledClassName(VarType type)
//{
//    auto v = getVarType(type);
//    if (v.typeName != "") type=v;
//    if (is_builtin_type(type.typeName)) return type.typeName;
//    auto name = VarType::mangle(type);
//    auto c = getClass(name);
//    if (c.type.typeName != "") return name;
//    if(type.templateArgs.size()!=0)
//    {
//        auto t = getClassTemplate(type.typeName);
//        //parser_.InstantiateTemplate(type,t);
//        return name;
//    }
//    return "";
//}

void SymbolTable::callDestructorForCurScope(BlockExprAST* block)
{
    auto exprlist = context_.callDestructorsOfScope();
    block->instructions().insert(block->instructions().end(), std::make_move_iterator(exprlist.begin()),
        std::make_move_iterator(exprlist.end()));
}

//std::vector<std::unique_ptr<ExprAST>> SymbolTable::callDestructor()
//{
//    std::vector<std::unique_ptr<ExprAST>> exprs;
//    auto valmap = named_values_.rbegin();
//    for (auto scope = named_values_seq_.rbegin();
//        scope != named_values_seq_.rend(); ++scope,++valmap)
//    {
//        for (auto it = scope->rbegin(); it != scope->rend(); ++it) {
//            auto& v = (*valmap)[*it];
//            if (!is_builtin_type(v.type.typeName))
//                exprs.push_back(parser_.callDestructor(v));
//        }
//    }
//    return exprs;
//}
//
//void SymbolTable::callNamelessVarDestructor(std::vector<std::unique_ptr<ExprAST>>& exprs)
//{
//    for(auto&v:nameless_values_)
//    {
//        exprs.push_back(parser_.callDestructor(v));
//    }
//    nameless_values_.clear();
//}


void SymbolTable::setSpecfiedNamespace(NamespaceHelper* ns) {
    specfied_namespace_ = ns;
}

void SymbolTable::unsetSpecfiedNamespace() {
    specfied_namespace_ = nullptr;
}

SymbolTable::ScopeGuard::ScopeGuard(SymbolTable& st): st_(st), block_(nullptr)
{
    st_.createScope();
}

SymbolTable::ScopeGuard::~ScopeGuard()
{
    if (block_ != nullptr)
        st_.callDestructorForCurScope(block_);
    st_.destroyScope();
}

void SymbolTable::ScopeGuard::setBlock(BlockExprAST* block)
{
    block_ = block;
}

SymbolTable::NamespaceGuard::NamespaceGuard(SymbolTable& st, const std::string& name): st_(st)
{
    st_.createNamespace(name);
}

SymbolTable::NamespaceGuard::~NamespaceGuard()
{
    st_.destroyNamespace();
}

std::vector<std::string> SymbolTable::getVariableListOfScope() {
    return named_values_seq_.back();
}

const std::vector<std::unique_ptr<Variable>>& SymbolTable::getNamelessVariableList() {
    return nameless_values_;
}

void SymbolTable::clearNamelessVariable() {
    nameless_values_.clear();
}

const std::vector<std::vector<std::string>>& SymbolTable::getVariableListOfAll() {
    return named_values_seq_;
}
