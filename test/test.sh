cmake .. &&
make &&
cp R-Cpp/R-Cpp ./compiler &&
./compiler src.rpp &&
clang++ driver.cpp output.o -lgtest -lpthread -o out &&
./out 