#include <iostream>
#include "../MathFunctions/MathFunctions.h"
#include "../StringUtilities/StringUtilities.h"

int main() {
    double num = 4;
    std::cout << "Square of " << num << ": " << square(num) << std::endl;
    std::cout << "Cube of " << num << ": " << cube(num) << std::endl;

    std::string testing = "palindrome_emordnilap";
    std::cout << "Length of \"" << testing << "\": " << countChars(testing) << std::endl;
    std::cout << (isPalindrome(testing) ? "\"" + testing + "\" is a palindrome." : "\"" + testing + "\" is not a palindrome.") << std::endl;

    return 0;
}
