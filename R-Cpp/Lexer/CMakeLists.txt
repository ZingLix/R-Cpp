cmake_minimum_required (VERSION 3.8)

file(GLOB src "*.cpp")
add_library(Lexer OBJECT  ${src})
