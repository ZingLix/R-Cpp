#include "AST.h"

Parse::CompoundStmt::CompoundStmt(std::vector<std::unique_ptr<Stmt>> exprs)
    : stmts_(std::move(exprs)) 
{ }

void Parse::CompoundStmt::print(std::string indent, bool last) {
    indent += last ? "  " : "| ";
    for (size_t i = 0; i < stmts_.size(); ++i) {
        stmts_[i]->print(indent, i == stmts_.size() - 1);
    }
}

Parse::IfStmt::IfStmt(std::unique_ptr<Stmt> condition, std::unique_ptr<CompoundStmt> then,
                      std::unique_ptr<CompoundStmt> els): cond_(std::move(condition)), then_(std::move(then)),
                                                          else_(std::move(els)) {
}

void Parse::IfStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-IfStmt " << std::endl;
    indent += last ? "  " : "| ";
    std::cout << indent << "+- Condition" << std::endl;
    cond_->print(indent, false);
    std::cout << indent << "+- Then" << std::endl;
    then_->print(indent, else_ == nullptr);
    if (else_) {
        std::cout << indent << "+- Else" << std::endl;
        else_->print(indent, true);
    }

}

Parse::ForStmt::ForStmt(std::unique_ptr<Stmt> start, std::unique_ptr<Stmt> cond, std::unique_ptr<Stmt> end,
                        std::unique_ptr<CompoundStmt> body): start_(std::move(start)), cond_(std::move(cond)),
                                                             end_(std::move(end)), body_(std::move(body)) {

}

void Parse::ForStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-ForStmt" << std::endl;
    indent += last ? "  " : "| ";
    std::cout << indent << "+-start" << std::endl;
    start_->print(indent + "| ", false);
    std::cout << indent << "+-cond" << std::endl;
    cond_->print(indent + "| ", false);
    std::cout << indent << "+-body" << std::endl;
    body_->print(indent + "| ", false);
    std::cout << indent << "+-end" << std::endl;
    end_->print(indent + "  ", true);
}

Parse::ReturnStmt::ReturnStmt(std::unique_ptr<Stmt> returnVal): ret_val_(std::move(returnVal)) {
}

void Parse::ReturnStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-ReturnStmt" << std::endl;
    indent += last ? "  " : " |";
    ret_val_->print(indent, true);
}

Parse::BinaryOperatorStmt::
BinaryOperatorStmt(std::unique_ptr<Stmt> lhs, std::unique_ptr<Stmt> rhs, OperatorType op): lhs_(std::move(lhs)),
                                                                                           rhs_(std::move(rhs)),
                                                                                           op_(op) {
}

void Parse::BinaryOperatorStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-BinaryOperatorStmt " << op_ << std::endl;
    indent += last ? "  " : "| ";
    lhs_->print(indent, false);
    rhs_->print(indent, true);
}

Parse::UnaryOperatorStmt::
UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op): stmt_(std::move(expr)), op_(op) {
}

Parse::UnaryOperatorStmt::UnaryOperatorStmt(std::unique_ptr<Stmt> expr, OperatorType op,
                                            std::vector<std::unique_ptr<Stmt>> args): stmt_(std::move(expr)), op_(op),
                                                                                      args_(std::move(args)) {
}

void Parse::UnaryOperatorStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-UnaryOperatorStmt " << op_ << std::endl;
    indent += last ? "  " : "| ";
    if (args_.size() == 0) {
        stmt_->print(indent, true);
    } else {
        stmt_->print(indent, false);
        for (size_t i = 0; i < args_.size(); ++i) {
            args_[i]->print(indent, i == args_.size() - 1);
        }
    }
}

Parse::VariableDefStmt::VariableDefStmt(const VarType& type, const std::string& name): type_(type), name_(name) {
}

void Parse::VariableDefStmt::setInitValue(std::unique_ptr<Stmt> initVal) {
    init_val_ = std::move(initVal);
}

void Parse::VariableDefStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-VariableDefStmt " << type_.typeName << " " << name_ << std::endl;
    indent += last ? "  " : "| ";
    if (init_val_)
        init_val_->print(indent, true);
}

Parse::NamelessVariableStmt::
NamelessVariableStmt(const VarType& type,
                     std::vector<std::unique_ptr<Stmt>> args): type_(type), args_(std::move(args)) {
}

void Parse::NamelessVariableStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-NamelessVariableStmt " << type_.typeName << std::endl;
    indent += last ? "  " : "| ";
    for (size_t i = 0; i < args_.size(); ++i) {
        args_[i]->print(indent, i == args_.size() - 1);
    }
}

Parse::VariableStmt::VariableStmt(const std::string& name): name_(name) {
}

void Parse::VariableStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-VariableStmt " << name_ << std::endl;
}

Parse::IntegerStmt::IntegerStmt(std::int64_t val): Stmt(VarType("i32")), val_(val) {
}

void Parse::IntegerStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-IntegerStmt " << val_ << std::endl;
}

Parse::FloatStmt::FloatStmt(double val): Stmt(VarType("double")), val_(val) {
}

void Parse::FloatStmt::print(std::string indent, bool last) {
    std::cout << indent << "+-FloatStmt " << val_ << std::endl;
}

Parse::FunctionDecl::FunctionDecl(::Function f, std::unique_ptr<CompoundStmt> body): func_(f), body_(std::move(body)) {
}

void Parse::FunctionDecl::print(std::string indent, bool last) {
    std::cout << indent << "+-Function " << func_.name <<" (";
    for (auto it = func_.args.begin(); it != func_.args.end();++it) {
        std::cout << it->type.typeName<<" "<<it->name;
        if (it != func_.args.end() - 1) std::cout << ",";
    }
    std::cout << ") -> ";
    std::cout << func_.returnType.typeName;
    if(func_.isExternal) {
        std::cout << " external" << std::endl;
    }else {
        std::cout << std::endl;
        body_->print(indent, last);
    }
    //indent += last ? "  " : "| ";

}

::Function Parse::FunctionDecl::getFunction() {
    return func_;
}

Parse::ClassDecl::ClassDecl(const std::string& name): name_(name) {
}

void Parse::ClassDecl::addMemberFunction(std::unique_ptr<FunctionDecl> func) {
    member_functions_.emplace_back(std::move(func));
}

void Parse::ClassDecl::addMemberVariable(VarType type, const std::string name) {
    member_variables_.emplace_back(type, name);
}

void Parse::ClassDecl::print(std::string indent, bool last) {
    std::cout << indent << "+-ClassDecl " << name_ << std::endl;
    indent += last ? "  " : "| ";
    std::cout << indent << "+-member variables" << std::endl;
    auto extraindent = member_functions_.empty() ? "  " : "| ";
    for (auto& p : member_variables_) {
        std::cout << indent << extraindent << "+-" << p.second << " " << p.first.typeName << std::endl;;
    }
    for (size_t i = 0; i < member_functions_.size(); ++i) {
        member_functions_[i]->print(indent, i == member_functions_.size() - 1);
    }
}
