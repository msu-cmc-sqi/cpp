#include "MathFunctions.h"
#include "StringUtilities.h"
#include <iostream>
#include <iomanip>

int main()
{
    double a = 5.2;
    int b = 7;
    std::string s1 = "abcdhdcba";
    std::string s2 = "abcddcba";
    std::string s3 = "acdhdcba";
    std::cout << std::fixed << std::setprecision(2) << "square: " << square(a) << ", cube = " << cube(b) << std::endl;
    std::cout << "Lenght1: " << countChars(s1) << ", Lenght2: " << countChars(s2) << std::endl;
    std::cout << "Is palindrom1: " << isPalindrome(s1) << ", Is palindrom2: " << isPalindrome(s2) << ", Is palindrom3: " << isPalindrome(s3)<<std::endl;
}
