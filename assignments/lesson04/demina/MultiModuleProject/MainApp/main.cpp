#include "MathFunctions.h"
#include "StringUtilities.h"

#include <iostream>
#include <string>
using namespace std;

int main() {
	int number = 13;
	cout << "Our check number = " << number << endl;
	cout << number << "^2 = " << square(number) << endl;
	cout << number << "^3 = " << cube(number) << endl;
	cout << "---" << endl;

	string str1 = "pop";
	cout << "Our first check string = " << str1 << endl;
	cout << "len: " << countChars(str1) << endl;
	cout << (isPalindrome(str1) ? "Это палиндром." : "Это не палиндром.") << endl;
	cout << "---" << endl;

	string str2 = "push";
	cout << "Our second check string = " << str2 << endl;
	cout << "len: " << countChars(str2) << endl;
	cout << (isPalindrome(str2) ? "Это палиндром." : "Это не палиндром.") << endl;

	return 0;
}
