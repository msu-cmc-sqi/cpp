#include <iostream>
#include "../MathFunctions/MathFunctions.h"
#include "../StringUtilities/StringUtilities.h"

int main(){
	double x = 8.5;
	std::cout << "Квадрат числа " << x << ": " << square(x) << '\n';
	std::cout << "Куб числа " << x << ": " << cube(x) << '\n';

	std::string s = "nosoroson";
	std::cout << "Число символов в строке " << s << ":" << countChars(s) << '\n';
	if (isPalindrome(s)) std::cout << s << "палиндром" << '\n';
	else std::cout << s << "не палиндром" << '\n';

	return 0;
}
