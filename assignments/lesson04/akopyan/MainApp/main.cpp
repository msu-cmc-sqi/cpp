#include <iostream>
#include "MathFunctions.h"
#include "StringUtilities.h"

using namespace std;

void testMathFunctions() {
    cout << "Тесты для MathFunctions:" << endl;

    cout << "Тест square(2): " << (square(2) == 4 ? "Верно" : "Неверно") << endl;
    cout << "Тест square(-3): " << (square(-3) == 9 ? "Верно" : "Неверно") << endl;
    cout << "Тест square(0): " << (square(0) == 0 ? "Верно" : "Неверно") << endl;

    cout << "Тест cube(2): " << (cube(2) == 8 ? "Верно" : "Неверно") << endl;
    cout << "Тест cube(-3): " << (cube(-3) == -27 ? "Верно" : "Неверно") << endl;
    cout << "Тест cube(0): " << (cube(0) == 0 ? "Верно" : "Неверно") << endl;

    cout << "Тесты для MathFunctions завершены." << endl << endl;
}

void testStringUtilities() {
    cout << "Тесты для StringUtilities:" << endl;

    cout << "Тест countChars(\"Привет\"): " << (countChars("Привет") == 6 ? "Верно" : "Неверно") << endl;
    cout << "Тест countChars(\"\"): " << (countChars("") == 0 ? "Верно" : "Неверно") << endl;
    cout << "Тест countChars(\" \"): " << (countChars(" ") == 1 ? "Верно" : "Неверно") << endl;

    cout << "Тест isPalindrome(\"шалаш\"): " << (isPalindrome("шалаш") ? "Верно" : "Неверно") << endl;
    cout << "Тест isPalindrome(\"level\"): " << (isPalindrome("level") ? "Верно" : "Неверно") << endl;
    cout << "Тест isPalindrome(\"hello\"): " << (!isPalindrome("hello") ? "Верно" : "Неверно") << endl;
    cout << "Тест isPalindrome(\"\"): " << (isPalindrome("") ? "Верно" : "Неверно") << endl;
    cout << "Тест isPalindrome(\"а\"): " << (isPalindrome("а") ? "Верно" : "Неверно") << endl;

    cout << "Тесты для StringUtilities завершены." << endl << endl;
}

int main() {
    testMathFunctions();
    testStringUtilities();
    return 0;
}