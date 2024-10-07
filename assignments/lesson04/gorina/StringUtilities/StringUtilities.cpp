#include "StringUtilities.h"

int countChars(std::string s) {
    int i = 0, count = 0;
    while (s[i++])
        count++;
    return count;
}
bool isPalindrome(std::string s) {
    int i = 0, j;
    bool result = true;
    j = s.size() - 1;
    while (true && i < j) {
        if (s[i] != s[j]) 
            result = false;
        i++;
        j--;
    }
    return result;
}
