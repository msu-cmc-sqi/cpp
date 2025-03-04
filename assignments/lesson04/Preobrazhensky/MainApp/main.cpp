#include <iostream>
#include <string>
#include "MathFunctions.h"
#include "StringUtilities.h"

int main(){
	std::string str;
	int h, g;
	std::cin >> h >> g >> str;
	std::cout << cube(h) << square(g) << std::endl;
	std::cout << countChars(str) << std::endl;
	if (isPalindrome(str)){
		std::cout << "TRUE" << std::endl;}
	return 0;
}
