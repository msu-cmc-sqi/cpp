//
//  StringUtilities.cpp
//
//  Created by Егор Мальцев on 05.10.2024.
//

#include "StringUtilities.h"

#include <string>

using namespace std;

unsigned long countChars(const string& str) {
    return str.length();
}

bool isPalindrome(const string& str) {
    return equal(str.begin(), str.begin() + str.size()/2, str.rbegin());
}
