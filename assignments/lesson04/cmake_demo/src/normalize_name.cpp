#include "normalize_name.hpp"

#include <stdexcept>

namespace lesson04 {

std::string normalize_name(std::string_view input) {
    const auto first = input.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) {
        throw std::invalid_argument("имя не должно быть пустым");
    }
    const auto last = input.find_last_not_of(" \t\n\r");
    return std::string(input.substr(first, last - first + 1));
}

} // namespace lesson04
