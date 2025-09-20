#include <iostream>
#include <nlohmann/json.hpp>   

using json = nlohmann::json;

int main() {
    // Создаём JSON-объект
    json j = {
        {"name", "Alice"},
        {"age", 25},
        {"is_active", true},
        {"skills", {"C++", "Python", "Java"}}
    };

    // Красиво печатаем JSON с отступами
    std::cout << j.dump(4) << std::endl;
    return 0;
}