#include "StringUtilities.h"

int countChars(const std::string& str)
{
    return str.length();
}

bool isPalindrome(const std::string& str)
{
    if (str.empty()) return true;
    
    auto start = str.begin();
    auto end = str.end() - 1;
    
    while (start != end && *start == *end) {
        ++start;
        --end;
    }
    
    return start >= end;
}
