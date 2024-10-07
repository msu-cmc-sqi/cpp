#include <iostream>
#include "../MathFunctions/MathFunctions.h"
#include "../StringUtilities/StringUtilities.h"

int main(){
	double sex = 6.9;
	std::cout << "square of number " << sex << ": " << square(sex) << '\n';
	std::cout << "tripple power of number  " << sex << ": " << cube(sex) << '\n';

	std::string sexy = "dovod";
	std::cout << "Number of chars in str " << sexy << ":" << countChars(sexy) << '\n';
	if (isPalindrome(sexy)) std::cout << sexy << "palindrom" << '\n';
	else std::cout << sexy << "bruh :(" << '\n';

	return 0;
}
