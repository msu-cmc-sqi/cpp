// test10_uninitialized_use.cpp
#include <iostream>

int main() {
    int x;
    if (x == 0) { // use of uninitialized variable
        std::cout << "zero\n";
    }
    return 0;
}

