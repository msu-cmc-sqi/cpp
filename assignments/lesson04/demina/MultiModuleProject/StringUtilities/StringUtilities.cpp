#include "StringUtilities.h"

#include <iostream>
#include <cctype>
#include <string>

using namespace std;

size_t countChars(const string& s) {
	return s.size();
}

bool isPalindrome(const string& s) {
	auto id_left = s.begin();
	auto id_right = s.end() - 1;

	while (id_left < id_right) {
		while (id_left < id_right && !isalnum(*id_left)) {
			id_left++;
		}
		while (id_left < id_right && !isalnum(*id_right)) {
			id_right--;
		}

		if (tolower(*id_left) != tolower(*id_right)) {
			return false;
		}

		id_left++;
		id_right--;
	}
	return true;
}
