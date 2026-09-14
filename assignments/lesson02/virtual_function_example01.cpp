#include <iostream>
#include <memory>

class Animal {
public:
    virtual ~Animal() = default;

    virtual void speak() {  // Виртуальная функция
        std::cout << "Animal speaks\n";
    }
};

class Dog : public Animal {
public:
    void speak() override {  // Переопределение виртуальной функции
        std::cout << "Dog barks\n";
    }
};

class Cat : public Animal {
public:
    void speak() override {  // Переопределение виртуальной функции
        std::cout << "Cat meows\n";
    }
};

int main() {
    std::unique_ptr<Animal> a1 = std::make_unique<Dog>();
    std::unique_ptr<Animal> a2 = std::make_unique<Cat>();

    a1->speak();  // Вызывает Dog::speak
    a2->speak();  // Вызывает Cat::speak

    return 0;
}
