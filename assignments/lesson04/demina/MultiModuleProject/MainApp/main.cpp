#include "MathFunctions.h"
#include "StringUtilities.h"
#include <iostream>

using namespace std;

int main() {
	int number = 13;
	cout << number << "^2 = " << square(number) << endl;
	cout << number << "^3 = " << cube(number) << endl;

	string s = "Push";
	cout << "Word: " << s << endl;
	cout << "countChars for " << s << " = " << countChars(s) << endl;
	cout << s << (isPalindrome(s) ? " is palindrome" : " is not palindrome") << endl;

	string s0 = "Pop";
	cout << "Word: " << s0 << endl;
	cout << "countChars for " << s0 << " = " << countChars(s0) << endl;
	cout << s0 << (isPalindrome(s0) ? " is palindrome" : " is not palindrome") << endl;

	return 0;
}
