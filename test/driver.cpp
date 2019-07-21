#include "gtest/gtest.h"

extern "C"{
    int fibonacci(int);
    int sum(int);
    int precedence();
    int classMemberFunction(int);
    int array(int);
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

int Array(int a){
    int arr[10];
    for(int i=1;i<11;i=i+1){
        arr[i-1]=a*i;
    }
    int sum_=0;
    for(int j=0;j<10;j=j+1){
        sum_ += arr[j];
    }
    return sum_;
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

TEST(ARRAY, basic){
    for(int i=2;i<10;++i){
        EXPECT_EQ(array(i),Array(i));
    }
}

int main(int ac, char* av[])
{
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}