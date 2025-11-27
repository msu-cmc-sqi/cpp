// test4_missing_virtual_destructor.cpp
#include <iostream>

class Base {
public:
    ~Base() { std::cout << "Base dtor\n"; }
};

class Derived : public Base {
public:
    ~Derived() { std::cout << "Derived dtor\n"; }
};

int main() {
    Base* b = new Derived();
    delete b; // undefined behavior: Base dtor not virtual
    return 0;
}
