// test2_buffer_overflow.cpp
#include <cstring>
#include <iostream>

void copy_input(const char* s) {
    char buf[16];
    // unsafe: no bounds check
    std::strcpy(buf, s);
    std::cout << buf << "\n";
}

int main() {
    copy_input("This string is definitely longer than sixteen bytes");
    return 0;
}
