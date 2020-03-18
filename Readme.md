# R-Cpp

[![Build Status](https://travis-ci.com/ZingLix/R-Cpp.svg?branch=master)](https://travis-ci.com/ZingLix/R-Cpp) [![Github Actions Status](https://github.com/ZingLix/R-cpp/workflows/build/badge.svg)](https://github.com/ZingLix/R-Cpp)

C++ is awesome, but it has too many disadvantages inherited from C. Abandoning C is totally impossible for C++ now. Therefore, this project was born. I want to design a completely new programming language which is still as powerful as C++ but simpler and easier.

You can consider R-Cpp as Cpp Reborn or Cpp Remake whatever. The final name is not determined, so if you have any good idea, contact me.

The main idea is to abandon some bad ideas from C and add some new features. 

You can find some examples in the [test file](https://github.com/ZingLix/R-Cpp/blob/master/test/src.rpp)(all implemented) and my expectations [here](https://github.com/ZingLix/R-Cpp/wiki)(might not be implemented).

## Differences from C++

What you still will see in R-cpp

- Zero Cost Abstractions
- RAII
- NO Garbage Collection
- Automatic Type Deduction

What you will not see in R-cpp

- Macro  (template can do better)
- Header Files (modules instead, no more declaration and implementation isolated)
- Raw Pointer (smart pointer with ref count instead)

What's new

- Compile-time Reflection

## Installation

CMake and LLVM-10 are required. 

```
cmake .
make
```

## How to Use

```
./R-Cpp <filename>
clang output.o
```

The part of linking is still in progress, so you'll need to use clang/gcc to link the required library (libc).
