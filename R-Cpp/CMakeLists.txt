# CMakeList.txt : CMake project for R-Cpp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories(${LLVM_INCLUDE_DIR})
add_definitions(${LLVM_DEFINITIONS})
# Add source to this project's executable.
file(GLOB src "*.cpp")
add_subdirectory ("CodeGenerator")
add_subdirectory( "Lexer" )
add_subdirectory("Parser" )
add_subdirectory("Util")
add_executable(R-Cpp ${src})
target_link_libraries(R-Cpp CodeGenerator Lexer Parser Util LLVM-10)
# TODO: Add tests and install targets if needed.
