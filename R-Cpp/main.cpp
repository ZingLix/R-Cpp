#include "Parser.h"
#include <iostream>

using namespace std;

int main()
{
    Parser l("/home/zinglix/example.txt");
    l.MainLoop();
    CodeGenerator cg;
    for(auto& expr:l.AST()) {
        expr->generateCode(cg)->print(llvm::errs());
        cout << endl;
    }
    return 0;
}
