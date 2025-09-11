#include <iostream>

class Animal {
public:
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
    Animal* a1 = new Dog();
    Animal* a2 = new Cat();

    a1->speak();  // Вызывает Dog::speak
    a2->speak();  // Вызывает Cat::speak

    delete a1;
    delete a2;

    return 0;
}

