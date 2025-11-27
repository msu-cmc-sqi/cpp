// test3_use_after_free.cpp
#include <iostream>

int* create() {
    int* p = new int(42);
    delete p;
    return p; // returns dangling pointer
}

int main() {
    int* q = create();
    std::cout << *q << std::endl; // use-after-free
    return 0;
}

