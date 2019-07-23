cmake --build .. &&
cp ./../R-Cpp/R-Cpp . &&
./R-Cpp src.rpp &&
clang++ driver.cpp output.o -lgtest -lpthread -o out &&
./out 