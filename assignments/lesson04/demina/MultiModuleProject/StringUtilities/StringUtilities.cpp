#include "StringUtilities.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <cctype>

using namespace std;

size_t countChars(const string& str) {
	    return str.size();
}

bool isPalindrome(const string& s) {
	auto left = s.begin();
	auto right = s.end() - 1;

	while (left < right) {
		while (left < right && !isalnum(*left)) {
			left++;
		}
		while (left < right && !isalnum(*right)) {
			right--;
		}

		if (tolower(*left) != tolower(*right)) {
			return false;
		}

		left++;
		right--;
	}	

	return true;
}
