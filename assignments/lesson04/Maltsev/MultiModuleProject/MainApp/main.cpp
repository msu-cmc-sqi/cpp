//
//  main.cpp
//
//  Created by Егор Мальцев on 05.10.2024.
//

#include <stdio.h>
#include <iostream>

#include "MathFunctions.h"
#include "StringUtilities.h"


using namespace std;

int main() {
    
    cout << "square of 2: " << square(2) << endl;
    cout << "cube of 2: " << cube(2) << endl;
    
    
    cout << "len of 'hello' is " << countChars("hello") << endl;
    cout << "len of 'bye' is " << countChars("bye") << endl;
    cout << "'hello' " << (isPalindrome("hello") ? "is a palindrome" : "is NOT a palindrome") << endl;
    cout << "'abccba' " << (isPalindrome("abccba") ? "is a palindrome" : "is NOT a palindrome") << endl;
    
    
    return 0;
}
