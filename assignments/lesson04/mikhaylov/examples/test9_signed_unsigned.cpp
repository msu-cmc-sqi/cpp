// test9_signed_unsigned.cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1,2,3};
    for (unsigned i = 0; i <= v.size(); ++i) { // mixing signed/unsigned, off-by-one
        std::cout << v[i] << "\n";
    }
    return 0;
}

