#include "Lexer.h"
#include <iostream>

using namespace std;

int main()
{
    Lexer l("example.txt");
    auto t = l.nextToken();
    while(t.type!=TokenType::Eof)
    {
        std::cout << (int)t.type << "\t" << t.content << endl;
        if (t.type == TokenType::Unknown) cout << "Unknown char." << endl;
        t = l.nextToken();
    }
    return 0;
}
