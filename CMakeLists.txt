# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("R-Cpp")
set(CXX_STANDARD 17)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIR}")


# Include sub-projects.
add_subdirectory ("R-Cpp")
