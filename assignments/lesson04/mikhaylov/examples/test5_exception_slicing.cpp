// test5_exception_slicing.cpp
#include <iostream>
#include <stdexcept>

class MyError : public std::runtime_error {
public:
    MyError(): std::runtime_error("my error") {}
};

void throw_by_value() {
    throw MyError();
}

int main() {
    try {
        throw_by_value();
    } catch (std::exception e) { // slicing: should catch by reference
        std::cout << e.what() << std::endl;
    }
    return 0;
}
