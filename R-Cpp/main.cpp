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
//    Parser l("/home/zinglix/example.txt");
    l.MainLoop();
    CodeGenerator cg(l);
    cg.generate();
    return 0;
}
