#include "gtest/gtest.h"

extern "C"{
    int fibonacci(int);
    int sum(int);
    int precedence();
    int classMemberFunction(int);
}

int Fibonacci(int i){
    if(i==1) return 1;
    else if(i==2) return 1;
    else return Fibonacci(i-1)+Fibonacci(i-2);
}

int Sum(int a){
    int count=0;
    for(int i=0;i<a;i=i+1)
        count=count+i;
    return count;
}

TEST(Function, recursive){
    for(int i=2;i<20;++i){
        EXPECT_EQ(fibonacci(i),Fibonacci(i));
    }
}

TEST(Basic, for){
    for(int i=2;i<20;++i){
        EXPECT_EQ(sum(i),Sum(i));
    }
}

TEST(BASIC, precedence){
    EXPECT_EQ(precedence(),62);
}

TEST(CLASS, memberFunction){
    for(int i=2;i<10;++i){
        EXPECT_EQ(classMemberFunction(i),2*(10+i)*(10+i));
    }
}

int main(int ac, char* av[])
{
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}