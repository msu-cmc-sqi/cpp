#include "normalize_name.hpp"

#include <stdexcept>

int main() {
    if (lesson04::normalize_name("  Ada \t") != "Ada") {
        return 1;
    }
    try {
        lesson04::normalize_name(" \n ");
    } catch (const std::invalid_argument&) {
        return 0;
    }
    return 1;
}
