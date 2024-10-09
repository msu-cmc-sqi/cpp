#include "StringUtilities.h"

int countChars (std::string s)
{
    int i = 0, cnt = 0;
    while (s[i])
    {
        cnt++;
        i++;
    }
    return cnt;
}

bool isPalindrome(std::string s)
{
    bool res = true;
    int i = 0, j = s.size() - 1;
    while (i <= j)
    {
        if (s[i] != s[j])
        {
            res = false;
            break;
        }
        i++;
        j--;
    }
    return res;

}