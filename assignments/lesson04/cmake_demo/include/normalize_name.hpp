#pragma once

#include <string>
#include <string_view>

namespace lesson04 {

// Удаляет пробелы по краям; пустая строка после удаления недопустима.
std::string normalize_name(std::string_view input);

} // namespace lesson04
