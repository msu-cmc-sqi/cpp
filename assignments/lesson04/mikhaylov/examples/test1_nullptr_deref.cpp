// test1_nullptr_deref.cpp
#include <iostream>

int main() {
    int *p = nullptr;
    // bug: dereference without check
    std::cout << *p << std::endl;
    return 0;
}

