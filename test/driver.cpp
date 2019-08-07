#include "gtest/gtest.h"

extern "C"{
    int _R9fibonacciI3i32(int);
    int _R3sumI3i32(int);
    int _R10precedence();
    int _R19classMemberFunctionI3i32(int);
    int _R5arrayI3i32(int);
    int _R3ptrI3i32I3i32(int,int);
    int _R9TemplateA();
    int _R9TemplateB();
    int _R17classConstructor1();
    int _R17classConstructor2I3i32I3i32(int,int);
}

int fibonacci(int i){
    return _R9fibonacciI3i32(i);
}

int sum(int i){
    return _R3sumI3i32(i);
}

int precedence(){
    return _R10precedence();
}

int classMemberFunction(int i){
    return _R19classMemberFunctionI3i32(i);
}

int classConstructor1(){
    return _R17classConstructor1();
}

int classConstructor2(int a,int b){
    return _R17classConstructor2I3i32I3i32(a,b);
}

int array(int i){
    return _R5arrayI3i32(i);
}

int ptr(int a,int b){
    return _R3ptrI3i32I3i32(a,b);
}

int templateA(){
    return _R9TemplateA();
}

int templateB(){
    return _R9TemplateB();
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

TEST(CLASS,constructor){
    EXPECT_EQ(classConstructor1(),3);
    EXPECT_EQ(classConstructor2(1,5),6);
    EXPECT_EQ(classConstructor2(12,35),47);
}

TEST(ARRAY, basic){
    for(int i=2;i<10;++i){
        EXPECT_EQ(array(i),Array(i));
    }
}

TEST(POINTER, basic){
    EXPECT_EQ(ptr(33,55),33+55);
    EXPECT_EQ(ptr(82,255),82+255);
    EXPECT_EQ(ptr(68,3),68+3);
}

TEST(TEMPLATE, basic){
    EXPECT_EQ(templateA(),1);
    EXPECT_EQ(templateB(),2);
}

int main(int ac, char* av[])
{
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}