#include "StringUtilities.h"
#include <algorithm>

size_t countChars(const std::string& str) {
    return str.size();
}

bool isPalindrome(const std::string& str) {
    std::string reversed = str;
    std::reverse(reversed.begin(), reversed.end());
    return str == reversed;
}