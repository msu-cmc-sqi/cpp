// test8_incorrect_format.cpp
#include <cstdio>

int main() {
    int x = 42;
    // wrong format specifier (%lu instead of %d)
    std::printf("value=%lu\n", x);
    return 0;
}

