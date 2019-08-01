#include <iostream>
#include "Parser/Parser.h"
#include "CodeGenerator/CodeGenerator.h"

using namespace std;

int main(int argc,char **argv)
{
    if(argc==1) {
        cout << "Please input the source file." << endl;
        return -1;
    }
    Parse::Parser l(argv[1]); 
 //   Parse::Parser l("/home/zinglix/example.txt");
    l.MainLoop();
    CG::CodeGenerator cg(l);
    cg.generate();
    return 0;
}
