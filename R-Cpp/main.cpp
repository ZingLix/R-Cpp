#include "Parser.h"
#include <iostream>

using namespace std;

int main(int argc,char **argv)
{
    if(argc==1) {
        cout << "Please input the source file." << endl;
        return -1;
    }
    Parser l(argv[1]);
    l.MainLoop();
    CodeGenerator cg;
    for(auto& expr:l.AST()) {
        expr->generateCode(cg)->print(llvm::errs());
        cout << endl;
    }
    return 0;
}
