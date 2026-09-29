#include "normalize_name.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: name_cli <имя>\n";
        return 1;
    }
    try {
        std::cout << "Привет, " << lesson04::normalize_name(argv[1]) << "!\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
