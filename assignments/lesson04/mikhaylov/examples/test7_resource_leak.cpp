// test7_resource_leak.cpp
#include <fstream>
#include <string>

void write_file(const std::string& name) {
    std::ofstream f(name);
    f << "hello\n";
    // missing close() is ok for RAII, but check error handling
    // simulate early return without flush
    return;
}

int main() {
    write_file("out.txt");
    return 0;
}

